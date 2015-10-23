// Multigrid poisson solver

#ifndef SIM_MGSOLVE_HPP
#define SIM_MGSOLVE_HPP

#include <vector>
#include "tools/vectors.hpp"

struct MGLevel 
{
	float h;
	Vec2i dim;
};

class MultigridPoisson
{
public:
	enum class BC { None, Periodic, Dirichlet, Neumann };

	MultigridPoisson(const Vec2i& size);
	bool solve(float& residual, float tolerance, int maxIter);

	Vec2i size;
	std::vector<MGLevel> levels;
	
	float omega_opt = 0;
	float h0 = 0;
	
	int nu1 = 2;  // pre-smoothing steps
	int nu2 = 2;  // post-smoothing steps
	// convergence Linf
	// make symmetric = false

	float* h_u;
	float* h_b;
protected:
	void relax(int level, int iterations);
	void clearZero(int level);
	bool doFMG(float& residual, float tolerance, int maxIter);
};

#endif