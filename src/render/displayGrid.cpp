#include "render/displayGrid.hpp"
#include "render/window.hpp"
#include <sstream>
#include <GLFW/glfw3.h>

using namespace std;

DisplayGrid::DisplayGrid(CLQueue& queue, const Vec2i& maxSize, GLWindow& window) :
	window(window), maxSize(maxSize),
	fillKernel(queue, "display.cl", "displayGrid")
{	
	// set up GL buffers
	vbGrid.defineAttrib(0, GL_FLOAT, 1, 0);
	vbLines.defineAttrib(0, GL_FLOAT, 4, offsetof(LineVertex, pos));
	vbLines.defineAttrib(1, GL_FLOAT, 4, offsetof(LineVertex, color));
	
	// cl buffers
	vector<float> dummy(maxSize.x * maxSize.y);
	vbGrid.buffer.setData(dummy.data(), dummy.size()); // so that CL can read proper size
	clGrid = make_unique<CLVertexBuffer<cl_float> >(queue, vbGrid);

	// shaders
	auto gridVS = make_shared<VertexShader>("debug_grid.vert");
	auto gridGS = make_shared<GeometryShader>("debug_grid.geom");
	auto lineVS = make_shared<VertexShader>("draw_line.vert");
	auto flatFS = make_shared<FragmentShader>("flatshade.frag");
	gridShader = make_unique<ShaderProgram>(gridVS, flatFS, gridGS);
	lineShader = make_unique<ShaderProgram>(lineVS, flatFS);

	// register key handler
	window.keyHandlers.push_back(bind(&DisplayGrid::keyHandler, this, placeholders::_1));
}

void DisplayGrid::compute()
{
	if (curGrid < 0)
	{
		if (displayList.empty())
			return;
		curGrid = 0;
		changeGrid();
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

	Grid1f* grid = displayList[curGrid].grid;

	gridShader->use();
	vbGrid.bind();
	gridShader->setUniform(gridShader->uniform("mult"), mult);
	gridShader->setUniform(gridShader->uniform("method"), displayMethod);
	gridShader->setUniform(gridShader->uniform("sizeOuter"), grid->layout);
	gridShader->setUniform(gridShader->uniform("sizeInner"), grid->size);
	gridShader->setUniform(gridShader->uniform("scale"), Vec2(2.0f / grid->layout.x, 2.0f / grid->layout.y));
	glDrawArrays(GL_POINTS, 0, (GLsizei)grid->layout.x * grid->layout.y);

	lineShader->use();
	vbLines.bind();
	lineShader->setUniform(lineShader->uniform("scale"), Vec2(2.0f / grid->layout.x, 2.0f / grid->layout.y));
	glDrawArrays(GL_LINES, 0, vbLines.buffer.size);
}

bool DisplayGrid::keyHandler(int key)
{
	switch (key)
	{
	case GLFW_KEY_MINUS:
		curGrid = (curGrid - 1 + displayList.size()) % displayList.size();
		changeGrid();
		return true;
	case GLFW_KEY_EQUAL:
		curGrid = (curGrid + 1) % displayList.size();
		changeGrid();
		return true;
	case GLFW_KEY_LEFT_BRACKET:
		mult /= 2.0f;
		changeGrid();
		return true;
	case GLFW_KEY_RIGHT_BRACKET:
		mult *= 2.0f;
		changeGrid();
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

void DisplayGrid::changeGrid()
{
	if (curGrid < 0)
		return;

	// title text
	stringstream str;
	str << "Display grid '" << displayList[curGrid].name << "' scale: +/- " << 1.0f/mult << endl;
	window.setTitle(str.str());

	// grid lines
	Vec2i size = displayList[curGrid].grid->layout;
	vector<LineVertex> lines;
	if (size.x < 256 && size.y < 256) {
		for (int i = 0; i <= size.x; i++)
		{
			LineVertex v0 = { Vec4(0, 0, 0, 0), Vec2(i-0.5, -0.5) };
			LineVertex v1 = { Vec4(0, 0, 0, 0), Vec2(i-0.5, size.y - 0.5) };
			lines.push_back(v0);
			lines.push_back(v1);
		}
		for (int i = 0; i <= size.y; i++)
		{
			LineVertex v0 = { Vec4(0, 0, 0, 0), Vec2(-0.5, i - 0.5) };
			LineVertex v1 = { Vec4(0, 0, 0, 0), Vec2(size.y - 0.5, i - 0.5) };
			lines.push_back(v0);
			lines.push_back(v1);
		}
	}
	vbLines.buffer.setData(lines.data(), lines.size());
}