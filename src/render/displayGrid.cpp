#include "render/displayGrid.hpp"
#include "render/window.hpp"
#include <sstream>
#include <GLFW/glfw3.h>

using namespace std;

DisplayGrid::DisplayGrid(CLQueue& queue, const Vec2i& maxSize, GLWindow& window) :
	window(window), maxSize(maxSize),
	fillKernel(queue, "display.cl", "displayGrid"),
	gridShader(make_shared<VertexShader>("debug_grid.vs"),
			   make_shared<FragmentShader>("debug_grid.fs"),
	           make_shared<GeometryShader>("debug_grid.gs"))
{
	vector<float> dummy(maxSize.x * maxSize.y);
	vbGrid.defineAttrib(0, GL_FLOAT, 1, 0);
	vbGrid.buffer.setData(dummy.data(), dummy.size());

	clGrid = make_unique<CLVertexBuffer<cl_float> >(queue, vbGrid);

	window.keyHandlers.push_back(bind(&DisplayGrid::keyHandler, this, placeholders::_1));
}

void DisplayGrid::compute()
{
	if (curGrid < 0)
	{
		if (displayList.empty())
			return;
		curGrid = 0;
		updateTitle();
	}
	Grid1f* grid = displayList[curGrid].grid;
	const Vec2i& size = grid->layout;
	if (size.x > maxSize.x || size.y > maxSize.y)
		fatalError("display grid size exceeds maxSize");

	clGrid->acquire();
	grid->upload();
	fillKernel.setArg(0, grid->data.handle);
	fillKernel.setArg(1, clGrid->handle);
	fillKernel.enqueue(size.x * size.y);
	clGrid->release();
}

void DisplayGrid::render()
{
	if (curGrid < 0)
		return;
	gridShader.use();
	vbGrid.bind();
	
	Grid1f* grid = displayList[curGrid].grid;
	gridShader.setUniform(gridShader.uniform("mult"), mult);
	gridShader.setUniform(gridShader.uniform("method"), displayMethod);
	gridShader.setUniform(gridShader.uniform("sizeOuter"), grid->layout);
	gridShader.setUniform(gridShader.uniform("sizeInner"), grid->size);
	gridShader.setUniform(gridShader.uniform("scale"), Vec2(2.0f / grid->layout.x, 2.0f / grid->layout.y));
	glDrawArrays(GL_POINTS, 0, (GLsizei)grid->layout.x * grid->layout.y);
}

bool DisplayGrid::keyHandler(int key)
{
	switch (key)
	{
	case GLFW_KEY_MINUS:
		curGrid = (curGrid - 1 + displayList.size()) % displayList.size();
		updateTitle();
		return true;
	case GLFW_KEY_EQUAL:
		curGrid = (curGrid + 1) % displayList.size();
		updateTitle();
		return true;
	case GLFW_KEY_LEFT_BRACKET:
		mult /= 2.0f;
		updateTitle();
		return true;
	case GLFW_KEY_RIGHT_BRACKET:
		mult *= 2.0f;
		updateTitle();
		return true;
	case GLFW_KEY_0:
		displayMethod = (displayMethod + 1) % 2;
		return true;
	default:
		return false;
	}
}

void DisplayGrid::attach(Grid1f* grid, const string& name)
{
	displayList.push_back(DisplayGridInfo(grid, name));
}

void DisplayGrid::updateTitle()
{
	if (curGrid < 0)
		return;

	stringstream str;

	str << "Display grid '" << displayList[curGrid].name << "' scale: +/- " << 1.0f/mult << endl;
	window.setTitle(str.str());
}