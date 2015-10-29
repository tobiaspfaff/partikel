// 2D simulation grid

#ifndef SIM_GRID_HPP
#define SIM_GRID_HPP

#include "compute/computeMain.hpp"
#include "tools/vectors.hpp"

class GridBase {
public:
	GridBase(const Vec2i& size) : size(size) {}
	virtual ~GridBase() {}

	const Vec2i size;
};

class Grid1f : public GridBase {
public:
	Grid1f(const Vec2i& size, CLQueue& queue);
	void upload();
	void download();
	void allocHost();

	CLBuffer<cl_float> cl;
	std::vector<float> host;
};

#endif