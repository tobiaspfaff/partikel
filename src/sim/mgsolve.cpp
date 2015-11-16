#include "sim/mgsolve.hpp"
#include "tools/log.hpp"
#include "compute/opencl.hpp"
#include <algorithm>
#include <fstream>

using namespace std;

MGLevel::MGLevel(const Vec2i& size, float h, CLQueue& queue) :
	h(h), dim(size),
	u(size, 1, BufferType::Both, queue),
	r(size, 1, BufferType::Both, queue),
	b(size, 1, BufferType::Both, queue)
{
}

BC::BC(Type type, float value, float h) : type(type)
{
	if (type == Type::Dirichlet)
	{
		M = 1;
		ghost = 2*value;
	}
	else if (type == Type::Neumann)
	{
		M = -1;
		ghost = -h * value;
	}
}

inline bool isPowerOf2(int x)
{
	return x != 0 && (x & (x - 1)) == 0;
}

MultigridPoisson::MultigridPoisson(const Vec2i& size, float h0, CLQueue& queue)
{
	if (!isPowerOf2(size.x) || !isPowerOf2(size.y))
		fatalError("Multigrid solver only supports 2^n grids.");

	// optimal omega
	// Irad Yavneh. On red-black SOR smoothing in multigrid. SIAM J. Sci. Comput., 17(1):180-192, 1996.
	omega = 4 - 2 * sqrt(2);

	// get max levels
	Vec2i lSize = size;
	float h = h0;
	while (lSize.x > 4 && lSize.y > 4)
	{
		levels.emplace_back(new MGLevel(lSize, h, queue));
		lSize = Vec2i(lSize.x / 2, lSize.y / 2);
		h *= 2.0f;
	}
	cout << levels.size() << " levels generated" << endl;
}

bool MultigridPoisson::solve(float& residual, float tolerance)
{
	if (!doFMG(residual, tolerance)) 
	{
		clearZero(0);
		cout << "Warning: fmg did not converge, retrying with zeroed vector" << endl;
		if (!doFMG(residual, tolerance))
		{
			cout << "FMG did not converge" << endl;
			return false;
		}
	}
	return true;
}

void MultigridPoisson::clearZero(int level)
{
	levels[level]->u.clear();
}

void MultigridPoisson::applyBC(int level)
{
	Grid1f& u = levels[level]->u;
	Vec2i size = u.size;
	const int DX = 1;
	const int DY = u.stride();
	float valPosX = (level == 0) ? (bcPosX.ghost) : 0;
	float valPosY = (level == 0) ? (bcPosY.ghost) : 0;
	float valNegX = (level == 0) ? (bcNegX.ghost) : 0;
	float valNegY = (level == 0) ? (bcNegY.ghost) : 0;

	// x pos bnd
	float* ptr = u.ptr(size.x, 0);
	for (int j = 0; j < size.y; j++)
	{
		*ptr = valPosX - bcPosX.M * ptr[-DX];
		ptr += DY;
	}

	// x neg bnd
	ptr = u.ptr(-1, 0);
	for (int j = 0; j < size.y; j++)
	{
		*ptr = valNegX - bcNegX.M * ptr[DX];
		ptr += DY;
	}

	// y pos bnd
	ptr = u.ptr(0, size.y);
	for (int j = 0; j < size.x; j++)
	{
		*ptr = valPosY - bcPosY.M * ptr[-DY];
		ptr += DX;
	}

	// y neg bnd
	ptr = u.ptr(0, -1);
	for (int j = 0; j < size.x; j++)
	{
		*ptr = valNegY - bcNegY.M * ptr[DY];
		ptr += DX;
	}
	// corners
	ptr = u.ptr(-1, -1);
	*ptr = 0.5* (ptr[ DX] + ptr[ DY]);
	ptr = u.ptr(size.x, -1);
	*ptr = 0.5* (ptr[-DX] + ptr[ DY]);
	ptr = u.ptr(-1, size.y);
	*ptr = 0.5* (ptr[ DX] + ptr[-DY]);
	ptr = u.ptr(size.x, size.y);
	*ptr = 0.5* (ptr[-DX] + ptr[-DY]);
}

void MultigridPoisson::vcycle(int fine)
{
	float l2, linf;
	applyBC(fine);

	computeResidual(fine, linf, l2);
	cout << "V-Cycle Initial residual inf:" << linf << " l2: " << l2 << endl;
	
	// down
	for (int i = fine; i < levels.size()-1; i++)
	{
		relax(i, nu1, false);
		computeResidual(i, linf, l2);
		cout << "V-Cycle Down: level " << i << " residual inf:" << linf << " l2: " << l2 << endl;
		restrictResidual(i + 1);
		clearZero(i + 1);		
		applyBC(i + 1);
	}
	
	// solve coarsest
	relax(levels.size()-1, 2* (nu1 + nu2), false);
	computeResidual(levels.size() - 1, linf, l2);

	// up
	for (int i = levels.size() - 2; i >= fine; i--)
	{
		prolongV(i + 1);
		applyBC(i); 
		relax(i, nu2, true);
		computeResidual(i, linf, l2);
		cout << "V-Cycle Up: level " << i << " residual inf:" << linf << " l2: " << l2 << endl;
	}
}

bool MultigridPoisson::doFMG(float& residual, float tolerance)
{
	float linf, l2;

	applyBC(0);
	computeResidual(0, linf, l2);
	cout << "FMG Initial residual inf:" << linf << " l2: " << l2 << endl;

	// initialize all residuals
	for (int i = 0; i < levels.size() - 1; i++)
	{
		if (i>0)
			clearZero(i); 
		applyBC(i);
		computeResidual(i, l2, linf);
		restrictResidual(i+1);		
	}

	// fine level for v-cycle; start at coarsest
	for (int fine = levels.size() - 1; fine >= 0; fine--)
	{
		// do a number of v-cycles
		int numV = nuV; // OpenCurrent uses (fine==0) ? nuV+1 : 1
		numV = (fine == 0) ? nuV+1 : 1;
		for (int nv = 0; nv < numV; nv++)
		{
			vcycle(fine);
		}

		// use as initial condition for next-finer level
		if (fine > 0)
		{
			prolongV(fine);
			//applyBC(fine-1); applied at the beginning of vcycle
		}
	}
	return true;
}

void MultigridPoisson::computeResidual(int level, float& linf, float& l2)
{
	MGLevel& l= *levels[level];
	const int DX = 1;
	const int DY = l.u.stride();
	const float h2Inv = 1.0f / sq(l.h);
	linf = 0;
	double l2d = 0;

	for (int j = 0; j < l.dim.y; j++)
	{
		float* ptrB = l.b.ptr(0, j);
		float* ptrU = l.u.ptr(0, j);
		float* ptrR = l.r.ptr(0, j);
		
		for (int i = 0; i < l.dim.x; i++) {
			float residual = (*ptrB) - h2Inv * (ptrU[-DX] + ptrU[DX] + ptrU[-DY] + ptrU[DY] - 4 * (*ptrU));
			*ptrR = residual;
			linf = max(linf, fabs(residual));
			l2d += sq(residual);
			ptrB++;
			ptrR++;
			ptrU++;
		}
	}
	l2d /= l.dim.x * l.dim.y;
	l2 = (float)l2d;
}

void MultigridPoisson::prolongV(int level)
{
	Grid1f& srcGrid = levels[level]->u;
	Grid1f& dstGrid = levels[level - 1]->u;
	int DX_SRC = 1;
	int DY_SRC = srcGrid.stride();
	int DX_DST = 1;
	int DY_DST = dstGrid.stride();
	
	const float c0 = 9.0f / 16.0f, c1 = 3.0f / 16.0f, c2 = 1.0f / 16.0f;

	for (int j = 0; j < srcGrid.size.y; j++)
	{
		float* src = srcGrid.ptr(0, j); 
		float* dst = dstGrid.ptr(0, 2*j);
		
		for (int i = 0; i < srcGrid.size.x; i++) {
			float v0 = c0 * src[0];
			dst[0]             += v0 + c1 * (src[-DX_SRC] + src[-DY_SRC]) + c2 * src[-DX_SRC - DY_SRC];
			dst[DX_DST]        += v0 + c1 * (src[ DX_SRC] + src[-DY_SRC]) + c2 * src[ DX_SRC - DY_SRC];
			dst[DY_DST]        += v0 + c1 * (src[-DX_SRC] + src[ DY_SRC]) + c2 * src[-DX_SRC + DY_SRC];
			dst[DX_DST+DY_DST] += v0 + c1 * (src[ DX_SRC] + src[ DY_SRC]) + c2 * src[ DX_SRC + DY_SRC];

			dst += 2;
			src ++;
		}
	}
}

void MultigridPoisson::restrictResidual(int level)
{
	Grid1f& srcGrid = levels[level - 1]->r;
	Grid1f& dstGrid = levels[level]->b;
	int DX_SRC = 1;
	int DY_SRC = srcGrid.stride();
	int DX_DST = 1;
	int DY_DST = dstGrid.stride();
	
	for (int j = 0; j < dstGrid.size.y; j++)
	{
		float* src = srcGrid.ptr(0, 2 * j);
		float* dst = dstGrid.ptr(0, j);
		
		for (int i = 0; i < dstGrid.size.x; i++) {
			*dst = 0.25 * (src[0] + src[DX_SRC] + src[DY_SRC] + src[DX_SRC + DY_SRC]);
			dst++;
			src += 2;
		}
	}
}

void MultigridPoisson::relaxCPU(Grid1f& u, Grid1f& b, const Vec2i& size, float h, int redBlack)
{
	const float h2 = sq(h);
	int DX = 1;
	int DY = u.stride();

	for (int j = 0; j < size.y; j++)
	{
		int i_start = (j + redBlack) % 2;
		float* ptrB = b.ptr(i_start, j);
		float* ptrU = u.ptr(i_start, j);
		float M0 = 4;
		if (j == 0)
			M0 += bcNegY.M;
		else if (j == size.y - 1)
			M0 += bcPosY.M;

		for (int i = i_start; i < size.x; i += 2) {
			float M = M0;
			if (i == 0)
				M += bcNegX.M;
			if (i == size.x - 1)
				M += bcPosX.M;

			float u0 = *ptrU;
			float eq = ptrU[-DX] + ptrU[DX] + ptrU[-DY] + ptrU[DY] - h2 * (*ptrB) - 4 * u0;
			*ptrU = u0 + omega * eq / M; // SOR
			ptrB += 2;
			ptrU += 2;
		}
	}
}

void MultigridPoisson::relax(int level, int iterations, bool reverse)
{
	for (int iters = 0; iters < iterations; iters++) 
	{
		for (int redBlack = 0; redBlack < 2; redBlack++) 
		{
			relaxCPU(levels[level]->u, levels[level]->b, levels[level]->dim, levels[level]->h, reverse ? (1 - redBlack) : redBlack);
		}
		applyBC(level);
	}	
}
