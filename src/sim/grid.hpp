// 2D simulation grid

#ifndef SIM_GRID_HPP
#define SIM_GRID_HPP

#include "compute/computeMain.hpp"
#include "tools/vectors.hpp"

class GridBase {
public:
	GridBase(const Vec2i& size, int ghost, const Vec2i& layout, BufferType type) : 
		type(type), size(size), layout(layout), ghost(ghost) {}
	virtual ~GridBase() {}

	BufferType type;
	const Vec2i size, layout;
	int ghost;
};

class Grid1f : public GridBase {
public:
	Grid1f(const Vec2i& size, int ghost, BufferType type, CLQueue& queue);
	void upload();
	void download();
	void clear();

	inline float* ptr() { return &data.buffer[ghost + ghost*layout.x]; }
	inline float* ptr(int x, int y) { return &data.buffer[(ghost+x) + (ghost+y)*layout.x]; }
	inline int stride() { return layout.x; }
	
	CLBuffer<cl_float> data;
};

class GridMac2f : public GridBase {
public:
	GridMac2f(const Vec2i& size, int ghost, BufferType type, CLQueue& queue);
	void upload();
	void download();
	
	inline float* ptrU() { return &u.buffer[ghost + ghost*layout.x]; }
	inline float* ptrV() { return &v.buffer[ghost + ghost*layout.x]; }
	inline float* ptrU(int x, int y) { return &u.buffer[(ghost + x) + (ghost + y)*layout.x]; }
	inline float* ptrV(int x, int y) { return &v.buffer[(ghost + x) + (ghost + y)*layout.y]; }
	inline int index(int x, int y) { return (ghost + x) + (ghost + y)*layout.x; }
	inline int stride() { return layout.x; }

	CLBuffer<cl_float> u, v;
};


#endif