#include "sim/pressure.hpp"

using namespace std;

static void set_mac_bc(GridMac2f& grid)
{
	const Vec2 bc(0, 0);
	const int DX = 1;
	const int DY = grid.stride();
	const int NX = grid.size.x;
	const int NY = grid.size.y;

	// u:x bnd
	float* ptr = grid.ptrU(-1, -1);
	for (int j = 0; j < NY + 3; j++)
	{
		ptr[0] = bc.x;
		ptr[DX] = bc.x;
		ptr[DX*(NX + 1)] = bc.x;
		ptr[DX*(NX + 2)] = bc.x;
		ptr += DY;
	}

	// v:y bnd
	ptr = grid.ptrV(-1, -1);
	for (int j = 0; j < NX + 3; j++)
	{
		ptr[0] = bc.y;
		ptr[DY] = bc.y;
		ptr[DY*(NY + 1)] = bc.y;
		ptr[DY*(NY + 2)] = bc.y;
		ptr += DX;
	}

	// u:y bnd
	ptr = grid.ptrU(1, -1);
	for (int j = 2; j <= NX; j++)
	{
		ptr[0] = ptr[DY];
		ptr[DY*(NY + 1)] = ptr[DY*NY];
		ptr += DX;
	}

	// v:x bnd
	ptr = grid.ptrV(-1, 1);
	for (int j = 2; j <= NY; j++)
	{
		ptr[0] = ptr[DX];
		ptr[DX*(NX + 1)] = ptr[DX*NX];
		ptr += DY;
	}
}


PressureSolver::PressureSolver(GridMac2f& vel, float h, CLQueue& queue) :
	solver(vel.size, h, queue), size(vel.size), vel(vel), h(h)
{
	solver.nuV = 2;
	BC bc(BC::Type::Neumann, 0, h); // forced inflow variable slip
	solver.bcNegX = bc;
	solver.bcNegY = bc;
	solver.bcPosX = bc;
	solver.bcPosY = bc;

	pressure = &solver.getU0();
	divergence = &solver.getB0();	
}

void PressureSolver::solve()
{
	float residual;
	set_mac_bc(vel);
	computeDivergence();
	solver.solve(residual, 0);
	correctVelocity();
	set_mac_bc(vel);
	computeDivergence(); // just for display
}

void PressureSolver::computeDivergence()
{
	const int DX = 1;
	const int DY = vel.stride();
	const float invh = 1.0f / h;
	double total = 0;

	for (int j = 0; j < size.y; j++)
	{
		float* ptrU = vel.ptrU(0, j);
		float* ptrV = vel.ptrV(0, j);
		float* ptrD = divergence->ptr(0, j);
		
		for (int i = 0; i < size.x; i++)
		{
			float div = invh * (ptrU[DX] - ptrU[0] + ptrV[DY] - ptrV[0]);
			*ptrD = div;
			total += fabs(div);
			ptrU++;
			ptrV++;
			ptrD++;
		}
	}
	cout << "Total divergence: " << total << endl;
}

void PressureSolver::correctVelocity()
{
	const int DX = 1;
	const int DY = pressure->stride();
	const float invh = 1.0f / h;
	
	for (int j = 0; j < size.y; j++)
	{
		float* ptrU = vel.ptrU(0, j);
		float* ptrV = vel.ptrV(0, j);
		float* ptrP = pressure->ptr(0, j);

		for (int i = 0; i < size.x; i++)
		{
			*ptrU -= invh * (ptrP[0] - ptrP[-DX]);
			*ptrV -= invh * (ptrP[0] - ptrP[-DY]);
			ptrU++;
			ptrV++;
			ptrP++;
		}
	}
}