// Pressure solver interface

#ifndef SIM_PRESSURE_HPP
#define SIM_PRESSURE_HPP

#include "sim/mgsolve.hpp"
#include "sim/grid.hpp"

class PressureSolver 
{
public:
	PressureSolver(GridMac2f& vel, float h, CLQueue& queue);

	void solve();

//protected:
	void computeDivergence();
	void correctVelocity();

	MultigridPoisson solver;
	Grid1f* divergence;
	Grid1f* pressure;
	GridMac2f& vel;
	const Vec2i& size;
	float h;
};

#endif