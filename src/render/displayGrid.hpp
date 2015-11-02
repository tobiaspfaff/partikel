// Debug display of simulation grids

#ifndef RENDER_DISPAYGRID_HPP
#define RENDER_DISPAYGRID_HPP

#include "sim/grid.hpp"
#include "render/shader.hpp"
#include "compute/computeMain.hpp"

class GLWindow;

struct DisplayGridInfo
{
	DisplayGridInfo(Grid1f* grid, const std::string& name) :
		grid(grid), name(name) {}

	Grid1f* grid;
	std::string name;
};

struct LineVertex
{
	Vec4 color;
	Vec2 pos;	
};

class DisplayGrid
{
public:
	DisplayGrid(CLQueue& queue, const Vec2i& maxSize, GLWindow& window);

	void attach(Grid1f* grid, const std::string& name);
	void render(); 
	void compute();
protected:
	void changeGrid();
	bool keyHandler(int key);

	GLWindow& window;
	Vec2i maxSize;
	std::vector<DisplayGridInfo> displayList;
	SingleVertexArray<cl_float> vbGrid;
	SingleVertexArray<LineVertex> vbLines;
	std::unique_ptr<CLVertexBuffer<cl_float> > clGrid;
	CLKernel fillKernel;
	std::unique_ptr<ShaderProgram> gridShader, lineShader;
	int curGrid = -1;
	int displayMethod = 0;
	float mult = 1.0f;
};

#endif