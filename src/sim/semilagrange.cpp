#include "sim/semilagrange.hpp"
#include "sim/pressure.hpp"

using namespace std;

/*
//! Semi-Lagrange interpolation kernel
KERNEL(bnd = 1) template<class T>
void SemiLagrange(FlagGrid& flags, MACGrid& vel, Grid<T>& dst, Grid<T>& src, Real dt, bool isLevelset, int orderSpace)
{
	// traceback position
	Vec3 pos = Vec3(i + 0.5f, j + 0.5f, k + 0.5f) - vel.getCentered(i, j, k) * dt;
	dst(i, j, k) = src.getInterpolatedHi(pos, orderSpace);
}

//! Semi-Lagrange interpolation kernel for MAC grids
KERNEL(bnd = 1)
void SemiLagrangeMAC(FlagGrid& flags, MACGrid& vel, MACGrid& dst, MACGrid& src, Real dt, int orderSpace)
{
	// get currect velocity at MAC position
	// no need to shift xpos etc. as lookup field is also shifted
	Vec3 xpos = Vec3(i + 0.5f, j + 0.5f, k + 0.5f) - vel.getAtMACX(i, j, k) * dt;
	Real vx = src.getInterpolatedComponentHi<0>(xpos, orderSpace);
	Vec3 ypos = Vec3(i + 0.5f, j + 0.5f, k + 0.5f) - vel.getAtMACY(i, j, k) * dt;
	Real vy = src.getInterpolatedComponentHi<1>(ypos, orderSpace);
	Vec3 zpos = Vec3(i + 0.5f, j + 0.5f, k + 0.5f) - vel.getAtMACZ(i, j, k) * dt;
	Real vz = src.getInterpolatedComponentHi<2>(zpos, orderSpace);

	dst(i, j, k) = Vec3(vx, vy, vz);
}
*/

inline float interpol(float* u, const Vec2i& size, int DY, const Vec2& pos) 
{
	int xi = (int)pos.x;
	int yi = (int)pos.y;
	float s1 = pos.x - (float)xi;
	float t1 = pos.y - (float)yi;
	
	// clamp to border
	if (pos.x < 0.) { xi = 0; s1 = 0.0; }
	if (pos.y < 0.) { yi = 0; t1 = 0.0; }
	if (xi >= size.x - 1) { xi = size.x - 2; s1 = 1.0; }
	if (yi >= size.y - 1) { yi = size.y - 2; t1 = 1.0; }
	const int DX = 1;
								
	float *p = &u[xi + DY * yi];
	float t0 = 1. - t1, s0 = 1. - s1;

	return (p[0]  * t0 + p[DY]      * t1) * s0
		 + (p[DX] * t0 + p[DX + DY] * t1) * s1;
}

void semiLagrangeSelfAdvect(GridMac2f& velSrc, GridMac2f& temp, float dt, float h)
{
	set_mac_bc(temp);
	set_mac_bc(velSrc);

	const int DX = 1;
	const int DY = velSrc.stride();
	float* u = velSrc.ptrU();
	float* v = velSrc.ptrV();
	float *uDst = temp.ptrU();
	float *vDst = temp.ptrV();
	float dth = dt / h;
	Vec2i clampSizeU(velSrc.size.x + 1, velSrc.size.y);
	Vec2i clampSizeV(velSrc.size.x, velSrc.size.y + 1);

	for (int j = 0; j < velSrc.size.y; j++)
	{
		int idx0 = j*DY;		
		for (int i = 0; i < velSrc.size.x; i++)
		{
			int idx = idx0 + i;
			Vec2 velAtX(u[idx], 0.25 * (v[idx] + v[idx + DY] + v[idx - DX] + v[idx - DX + DY]));
			Vec2 velAtY(0.25 * (u[idx] + u[idx + DX] + u[idx - DY] + u[idx + DX - DY]), v[idx]);

			Vec2 xpos = Vec2((float)i, (float)j) - dth * velAtX;
			Vec2 ypos = Vec2((float)i, (float)j) - dth * velAtY;
			
			uDst[idx] = interpol(u, clampSizeU, DY, xpos);
			vDst[idx] = interpol(v, clampSizeV, DY, ypos);
		}
	}
	velSrc.swap(temp);
}

