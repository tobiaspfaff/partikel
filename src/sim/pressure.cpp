#include "sim/pressure.hpp"

using namespace std;

static void set_mac_bc(float* u, float* v, const Vec2i& size)
{
	const Vec2 bc(0, 0);
	const int DX = 1;
	const int DY = size.x + 3;
	const int NX = size.x;
	const int NY = size.y;

	// u:x bnd
	float* ptr = &u[0];
	for (int j = 0; j < size.y + 3; j++)
	{
		ptr[0] = bc.x;
		ptr[DX] = bc.x;
		ptr[DX*(NX + 1)] = bc.x;
		ptr[DX*(NX + 2)] = bc.x;
		ptr += DY;
	}

	// v:y bnd
	ptr = &v[0];
	for (int j = 0; j < size.x + 3; j++)
	{
		ptr[0] = bc.y;
		ptr[DY] = bc.y;
		ptr[DY*(NY + 1)] = bc.y;
		ptr[DY*(NY + 2)] = bc.y;
		ptr += DX;
	}

	// u:y bnd
	ptr = &u[2 * DX];
	for (int j = 2; j <= size.x; j++)
	{
		ptr[0] = ptr[DY];
		ptr[DY*(NY + 1)] = ptr[DY*NY];
		ptr += DX;
	}

	// v:x bnd
	ptr = &v[2 * DY];
	for (int j = 2; j <= size.y; j++)
	{
		ptr[0] = ptr[DX];
		ptr[DX*(NX + 1)] = ptr[DX*NX];
		ptr += DY;
	}
}


PressureSolver::PressureSolver(GridMac2f& vel, float h) :
solver(vel.size, h), size(vel.size), vel(vel), h(h)
{
	solver.nuV = 15;
	BC bc(BC::Type::Neumann, 0, h); // forced inflow variable slip
	solver.bcNegX = bc;
	solver.bcNegY = bc;
	solver.bcPosX = bc;
	solver.bcPosY = bc;

	pressure = solver.getU0();
	divergence = solver.getB0();	
}

void PressureSolver::solve()
{
	float residual;
	set_mac_bc(&vel.u[0], &vel.v[0], size);
	computeDivergence();
	solver.solve(residual, 0);
	computeDivergence(); // just for display
	correctVelocity();
	set_mac_bc(&vel.u[0], &vel.v[0], size);
}

void PressureSolver::computeDivergence()
{
	const int DX = 1;
	const int DY = size.x + 3;
	const float invh = 1.0f / h;
	double total = 0;

	for (int j = 0; j < size.y; j++)
	{
		float* ptrU = &vel.u[DX + (j + 1)*DY];
		float* ptrV = &vel.v[DX + (j + 1)*DY];
		float* ptrD = &divergence[DX + (j + 1)*DY];

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
	const int DY = size.x + 3;
	const float invh = 1.0f / h;
	
	for (int j = 0; j < size.y; j++)
	{
		float* ptrU = &vel.u[DX + (j + 1)*DY];
		float* ptrV = &vel.v[DX + (j + 1)*DY];
		float* ptrP = &pressure[DX + (j + 1)*DY];

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