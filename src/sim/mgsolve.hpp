// Multigrid poisson solver

#ifndef SIM_MGSOLVE_HPP
#define SIM_MGSOLVE_HPP

#include <vector>
#include <memory>
#include "tools/vectors.hpp"
#include "sim/grid.hpp"

struct MGLevel 
{
	MGLevel(const Vec2i& size, float h, CLQueue& queue);

	float h;
	Vec2i dim;
	Grid1f u, b, r;
};

struct BC 
{
	enum class Type { None = 0, Dirichlet, Neumann };
	BC() {}
	BC(Type type, float value, float h);

	Type type = Type::None;
	float ghost = 0;
	float M = 0;
};

class MultigridPoisson
{
public:
	
	MultigridPoisson(const Vec2i& size, float h, CLQueue& queue);
	bool solve(float& residual, float tolerance);
	inline Grid1f& getB0() { return levels[0]->b; }
	inline Grid1f& getU0() { return levels[0]->u; }

	std::vector<std::unique_ptr<MGLevel> > levels;
	
	float omega = 0;
	
	int nu1 = 2;  // pre-smoothing steps
	int nu2 = 2;  // post-smoothing steps
	int nuV = 2; // number of v-cycles in FMG

	BC bcPosX, bcNegX, bcPosY, bcNegY;

//protected:
	void computeResidual(int level, float& linf, float& l2);
	void restrictResidual(int level);
	void prolongV(int level);
	void relax(int level, int iterations, bool reverse);
	void clearZero(int level);
	bool doFMG(float& residual, float tolerance);
	void vcycle(int fine);
	void applyBC(int level);
	void relaxCPU(Grid1f& u, Grid1f& b, const Vec2i& size, float h, int redBlack);
};

#endif