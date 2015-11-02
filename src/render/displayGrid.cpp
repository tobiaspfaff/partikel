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
	vbGridLines.defineAttrib(0, GL_FLOAT, 4, offsetof(LineVertex, pos));
	vbGridLines.defineAttrib(1, GL_FLOAT, 4, offsetof(LineVertex, color));
	vbVelLines.defineAttrib(0, GL_FLOAT, 4, offsetof(LineVertex, pos));
	vbVelLines.defineAttrib(1, GL_FLOAT, 4, offsetof(LineVertex, color));

	// cl buffers
	vector<float> dummy(maxSize.x * maxSize.y);
	vbGrid.buffer.setData(dummy.data(), dummy.size()); // so that CL can read proper size
	clGrid = make_unique<CLVertexBuffer<cl_float> >(queue, vbGrid);

	// shaders
	auto gridVS = make_shared<VertexShader>("real_grid.vert");
	auto gridGS = make_shared<GeometryShader>("real_grid.geom");
	auto lineVS = make_shared<VertexShader>("draw_line.vert");
	auto flatFS = make_shared<FragmentShader>("flatshade.frag");
	gridShader = make_unique<ShaderProgram>(gridVS, flatFS, gridGS);
	lineShader = make_unique<ShaderProgram>(lineVS, flatFS);

	// register key handler
	window.keyHandlers.push_back(bind(&DisplayGrid::keyHandler, this, placeholders::_1, placeholders::_2));
}

void DisplayGrid::compute()
{
	if (curRealGrid < 0 && !displayRealList.empty())
	{
		curRealGrid = 0;
		changeGrid();
	}
	
	if (curRealGrid >= 0)
	{
		Grid1f* grid = displayRealList[curRealGrid].grid;
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
	if (curVelGrid >= 0)
	{
		GridMac2f* grid = displayVelList[curVelGrid].grid;
		const Vec2i& size = grid->size;
		int idx = 0;
		vector<LineVertex> lines;
		if (velCentered)
		{
			lines.resize(2 * (size.x + 2)*(size.y + 2));
			Vec4 color(0, 0.5, 0, 1);
			for (int j = 0; j < size.y + 2; j++)
			{
				float* ptrU = grid->ptrU(-1, j - 1);
				float* ptrV = grid->ptrV(-1, j - 1);
				for (int i = 0; i < size.x + 2; i++)
				{
					Vec2 dv(*ptrU + ptrU[grid->stride()], *ptrV + ptrV[grid->stride()]);
					dv *= 0.5f * multVel;
					Vec2 pos((float)i, (float)j);
					lines[idx++] = { color, pos };
					lines[idx++] = { color, pos + dv };
					ptrU++;
					ptrV++;
				}
			}
		}
		else {
			lines.resize(2 * (size.x + 2)*(size.y + 3) + 2 * (size.x + 3)*(size.y + 2));
			Vec4 color(0.5, 0, 0, 1);
			for (int j = 0; j < size.y + 2; j++)
			{
				float* ptr = grid->ptrU(-1, j - 1);
				for (int i = 0; i < size.x + 3; i++)
				{
					float vx = *ptr * multVel;
					Vec2 pos((float)i - 0.5f, (float)j);
					lines[idx++] = { color, pos };
					lines[idx++] = { color, pos + Vec2(vx, 0) };
					ptr++;
				}
			}
			color = Vec4(0, 0, 0.5, 1);
			for (int j = 0; j < size.y + 3; j++)
			{
				float* ptr = grid->ptrV(-1, j - 1);
				for (int i = 0; i < size.x + 2; i++)
				{
					float vy = *ptr * multVel;
					Vec2 pos((float)i, (float)j - 0.5f);
					lines[idx++] = { color, pos };
					lines[idx++] = { color, pos + Vec2(0, vy) };
					ptr++;
				}
			}
		}
		vbVelLines.buffer.setData(lines.data(), lines.size());
	}
}

void DisplayGrid::render()
{
	// draw real grid
	if (curRealGrid >= 0)
	{
		Grid1f* grid = displayRealList[curRealGrid].grid;
		gridShader->use();
		vbGrid.bind();
		gridShader->setUniform(gridShader->uniform("mult"), multReal);
		gridShader->setUniform(gridShader->uniform("method"), displayMethod);
		gridShader->setUniform(gridShader->uniform("sizeOuter"), grid->layout);
		gridShader->setUniform(gridShader->uniform("sizeInner"), grid->size);
		gridShader->setUniform(gridShader->uniform("scale"), Vec2(2.0f / grid->layout.x, 2.0f / grid->layout.y));
		glDrawArrays(GL_POINTS, 0, (GLsizei)grid->layout.x * grid->layout.y);

		// draw grid lines
		lineShader->use();
		vbGridLines.bind();
		lineShader->setUniform(lineShader->uniform("scale"), Vec2(2.0f / grid->layout.x, 2.0f / grid->layout.y));
		glDrawArrays(GL_LINES, 0, vbGridLines.buffer.size);
	}

	// draw velocity lines
	if (curVelGrid >= 0)
	{
		GridMac2f* grid = displayVelList[curVelGrid].grid;
		lineShader->use();
		vbVelLines.bind();
		lineShader->setUniform(lineShader->uniform("scale"), Vec2(2.0f / (grid->layout.x-1), 2.0f / (grid->layout.y-1)));
		glDrawArrays(GL_LINES, 0, vbVelLines.buffer.size);
	}
}

bool DisplayGrid::keyHandler(int key, int mods)
{
	switch (key)
	{
	case GLFW_KEY_MINUS:
		curVelGrid++;
		if (curVelGrid == displayVelList.size())
			curVelGrid = -1;
		changeGrid();
		return true;
	case GLFW_KEY_EQUAL:
		curRealGrid = (curRealGrid + 1) % displayRealList.size();
		changeGrid();
		return true;
	case GLFW_KEY_LEFT_BRACKET:
		if (mods & 1)
			multVel /= 2.0f;
		else
			multReal /= 2.0f;
		changeGrid();
		return true;
	case GLFW_KEY_RIGHT_BRACKET:
		if (mods & 1)
			multVel *= 2.0f;
		else
			multReal *= 2.0f;
		changeGrid();
		return true;
	case GLFW_KEY_0:
		displayMethod = (displayMethod + 1) % 2;
		return true;
	case GLFW_KEY_9:
		velCentered = (velCentered + 1) % 2;
		return true;
	default:
		return false;
	}
}

void DisplayGrid::attach(Grid1f* grid, const string& name)
{
	displayRealList.push_back(DisplayGridInfo(grid, name));
}

void DisplayGrid::attach(GridMac2f* grid, const string& name)
{
	displayVelList.push_back(DisplayVelInfo(grid, name));
}

void DisplayGrid::changeGrid()
{
	string realGrid = (curRealGrid >= 0) ? displayRealList[curRealGrid].name : "";
	string velGrid = (curVelGrid >= 0) ? displayVelList[curVelGrid].name : "";
	
	// title text
	stringstream str;
	str << "Real grid '" << realGrid << "' scale: +/- " << 1.0f / multReal << " ";
	if (!velGrid.empty())
	{
		str << "Vel grid '" << velGrid << "' scale: +/- " << 1.0f / multVel << " ";
	}
	window.setTitle(str.str());

	if (curRealGrid >= 0)
	{
		// grid lines
		Vec2i size = displayRealList[curRealGrid].grid->layout;
		vector<LineVertex> lines;
		if (size.x < 256 && size.y < 256) {
			for (int i = 0; i <= size.x; i++)
			{
				LineVertex v0 = { Vec4(0, 0, 0, 0), Vec2(i - 0.5, -0.5) };
				LineVertex v1 = { Vec4(0, 0, 0, 0), Vec2(i - 0.5, size.y - 0.5) };
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
		vbGridLines.buffer.setData(lines.data(), lines.size());
	}
}