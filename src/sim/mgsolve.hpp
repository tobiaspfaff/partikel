// Multigrid poisson solver

#ifndef SIM_MGSOLVE_HPP
#define SIM_MGSOLVE_HPP

#include <vector>
#include <memory>
#include "tools/vectors.hpp"

struct MGLevel 
{
	float h;
	Vec2i dim;
	std::vector<float> u;
	std::vector<float> b;
	std::vector<float> r;
};

struct BC 
{
	enum class Type { Dirichlet = 0, Neumann = 1 };
	Type type = Type::Dirichlet;
	float value = 0;
};

class MultigridPoisson
{
public:
	
	MultigridPoisson(const Vec2i& size);
	bool solve(float& residual, float tolerance, int maxIter);

	std::vector<std::unique_ptr<MGLevel> > levels;
	
	float omega_opt = 0;
	
	int nu1 = 2;  // pre-smoothing steps
	int nu2 = 2;  // post-smoothing steps
	int nuV = 2; // number of v-cycles in FMG

	BC bcPosX, bcNegX, bcPosY, bcNegY;

//protected:
	double computeResidual(int level);
	void restrictResidual(int level);
	void prolongV(int level);
	void relax(int level, int iterations);
	void clearZero(int level);
	bool doFMG(float& residual, float tolerance, int maxIter);
	void vcycle(int fine);
	void applyBC(int level, bool initialize);
};

#endif