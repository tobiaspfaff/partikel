#include "sim/grid.hpp"

using namespace std;

Grid1f::Grid1f(const Vec2i& size, int ghost, BufferType type, CLQueue& queue) :
	GridBase(size, ghost, size+Vec2i(2*ghost), type), 
	data(queue, layout.x*layout.y, type) 
{
}

void Grid1f::upload()
{
	data.upload();
}

void Grid1f::download()
{
	data.download();
}

void Grid1f::clear()
{
	if (type == BufferType::Host || type == BufferType::Both)
		fill(data.buffer.begin(), data.buffer.end(), 0.0f);
}


GridMac2f::GridMac2f(const Vec2i& size, int ghost, BufferType type, CLQueue& queue) :
	GridBase(size, ghost, size+Vec2i(2*ghost), type),
	u(queue, layout.x*layout.y, type),
	v(queue, layout.x*layout.y, type)
{
}

void GridMac2f::upload()
{
	u.upload();
	v.upload();	
}

void GridMac2f::download()
{
	u.download();
	v.download();
}

