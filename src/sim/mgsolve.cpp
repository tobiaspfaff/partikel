#include "sim/mgsolve.hpp"
#include "tools/log.hpp"
#include "compute/opencl.hpp"
#include <algorithm>

using namespace std;

inline bool isPowerOf2(int x)
{
	return x != 0 && (x & (x - 1)) == 0;
}

// Irad Yavneh. On red-black SOR smoothing in multigrid. SIAM J. Sci. Comput., 17(1):180-192, 1996.

MultigridPoisson::MultigridPoisson(const Vec2i& size) :
	size(size)
{
	if (!isPowerOf2(size.x) || !isPowerOf2(size.y))
		fatalError("Multigrid solver only supports 2^n grids.");

	Vec2 h0(100.0f / size.x, 100.0f / size.y); // grid spacing

	// get optimal omega
	c_i = Vec2(sq(h0.x) / norm2(h0), sq(h0.y) / norm2(h0));
	float C_max = 1.0 - min(c_i.x, c_i.y);
	omega_opt = 2.0f / (1.0f + sqrt(1 - sq(C_max)));

	// get max levels
	Vec2i lSize = size;
	while (lSize.x > 4 && lSize.y > 4)
	{
		MGLevel level;
		level.dim = lSize;
		level.h = min(h0.x, h0.y);
		levels.push_back(level);

		lSize /= 2;
		h0 *= 2.0f;
	}

	h_u = new float[size.x * size.y];
	h_b = new float[size.x * size.y];
}

bool MultigridPoisson::solve(float& residual, float tolerance, int maxIter)
{
	relax(0, 1);
	return true;

	if (!doFMG(residual, tolerance, maxIter)) 
	{
		clearZero(0);
		cout << "Warning: fmg did not converge, retrying with zeroed vector" << endl;
		if (!doFMG(residual, tolerance, maxIter))
		{
			cout << "FMG did not converge" << endl;
			return false;
		}
	}
	return true;
}

void MultigridPoisson::clearZero(int level)
{

}

bool MultigridPoisson::doFMG(float& residual, float tolerance, int maxIter) 
{
	return false;
}

// assume Neumann conditions
static void relaxCPU(float* h_u, float* h_b, const Vec2i& size, float h, int redBlack, float omega)
{
	const float h2 = sq(h);
	float* ptrB = redBlack ? (h_b + 1) : h_b;
	float* ptrU = redBlack ? (h_u + 1) : h_u;
	int DX = 1;
	int DY = size.x;

	for (int j = 0; j < size.y; j++) 
	{
		int i_start = (j + redBlack) % 2;
		
		for (int i = i_start; i < size.x; i += 2) {
			float L = 0;
			float residual = -h2 * (*ptrB);
			if (i > 0)
			{
				residual += ptrU[-DX];
				L++;
			}
			if (i < size.x)
			{
				residual += ptrU[DX];
				L++;
			}
			if (j > 0)
			{
				residual += ptrU[-DY];
				L++;
			}
			if (j < size.y)
			{
				residual += ptrU[DY];
				L++;
			}
			*ptrU += omega * residual / L;			
		}
	}
}

void MultigridPoisson::relax(int level, int iterations)
{
	for (int iters = 0; iters < iterations; iters++) 
	{
		for (int redBlack = 0; redBlack < 2; redBlack++) 
		{
			relaxCPU(h_u, h_b, size, h0, redBlack, omega_opt);
		}
	}
}
