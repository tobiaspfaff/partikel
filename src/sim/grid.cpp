#include "sim/grid.hpp"

using namespace std;

Grid1f::Grid1f(const Vec2i& size, CLQueue& queue) : 
	GridBase(size), cl(queue, size.x*size.y) 
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