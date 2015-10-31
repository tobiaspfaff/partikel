#include "sim/grid.hpp"

using namespace std;

Grid1f::Grid1f(const Vec2i& size, int ghost, CLQueue& queue) : 
	GridBase(size, size+Vec2i(2*ghost)), 
	cl(queue, layout.x*layout.y) 
{
}

void Grid1f::allocHost()
{
	host.resize(cl.size);
}

void Grid1f::upload()
{
	cl.write(host);
}

void Grid1f::download()
{
	cl.read(host);
}


GridMac2f::GridMac2f(const Vec2i& size, int ghost, CLQueue& queue) :
	GridBase(size, size+Vec2i(2*ghost)),
	clU(queue, layout.x*layout.y),
	clV(queue, layout.x*layout.y)
{
}

void GridMac2f::allocHost()
{
	u.resize(clU.size);
	v.resize(clV.size);
}

void GridMac2f::upload()
{
	clU.write(u);
	clV.write(v);
}

void GridMac2f::download()
{
	clU.read(u);
	clV.read(v);
}

