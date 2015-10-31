// 2D simulation grid

#ifndef SIM_GRID_HPP
#define SIM_GRID_HPP

#include "compute/computeMain.hpp"
#include "tools/vectors.hpp"

class GridBase {
public:
	GridBase(const Vec2i& size, const Vec2i& layout) : size(size), layout(layout) {}
	virtual ~GridBase() {}

	const Vec2i size, layout;
};

class Grid1f : public GridBase {
public:
	Grid1f(const Vec2i& size, int ghost, CLQueue& queue);
	void upload();
	void download();
	void allocHost();

	CLBuffer<cl_float> cl;
	std::vector<float> host;
};

class GridMac2f : public GridBase {
public:
	GridMac2f(const Vec2i& size, int ghost, CLQueue& queue);
	void upload();
	void download();
	void allocHost();

	CLBuffer<cl_float> clU, clV;
	std::vector<float> u, v;
};


#endif