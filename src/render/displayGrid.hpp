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

class DisplayGrid
{
public:
	DisplayGrid(CLQueue& queue, const Vec2i& maxSize, GLWindow& window);

	void attach(Grid1f* grid, const std::string& name);
	void render(); 
	void compute();
protected:
	void updateTitle();
	bool keyHandler(int key);

	GLWindow& window;
	Vec2i maxSize;
	std::vector<DisplayGridInfo> displayList;
	SingleVertexArray<cl_float> vbGrid;
	std::unique_ptr<CLVertexBuffer<cl_float> > clGrid;
	CLKernel fillKernel;
	ShaderProgram gridShader;
	int curGrid = -1;
	int displayMethod = 0;
	float mult = 1.0f;
};

#endif