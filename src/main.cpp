#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
#include "render/opengl.hpp"
#include "render/colors.hpp"
#include "render/window.hpp"
#include "tools/vectors.hpp"
#include "tools/log.hpp"
#include "compute/computeMain.hpp"
#include "sim/grid.hpp"
#include "sim/mgsolve.hpp"
#include "render/shader.hpp"
#include "render/texture.hpp"
#include "render/vertexArray.hpp"
#include <GLFW/glfw3.h>

using namespace std;

extern const char* git_version_short;

int displayGrid = 0;
int displayLevel = 0;
MultigridPoisson* _mg;

bool keyHandler(int key) 
{
	if (key == GLFW_KEY_MINUS)
	{
		displayLevel = max(0, displayLevel - 1);
	}
	else if (key == GLFW_KEY_EQUAL)
	{
		displayLevel = min((int)_mg->levels.size()-1, displayLevel + 1);
	}
	else if (key == GLFW_KEY_BACKSPACE)
	{
		displayGrid = (displayGrid+1) % 3;
		if (displayGrid == 0)
			cout << "Grid: U" << endl;
		if (displayGrid == 1)
			cout << "Grid: B" << endl;
		if (displayGrid == 2)
			cout << "Grid: R" << endl;
	}
	else if (key == GLFW_KEY_F)
	{
		float dummy;
		cout << "FMG cycle" << endl;
		//_mg->doFMG(dummy, 0);
	}
	else if (key == GLFW_KEY_V)
	{
		cout << "V cycle" << endl;
		//_mg->vcycle(0);
	}
	else if (key == GLFW_KEY_R)
	{
/*		cout << "100 relax steps" << endl;
		float l2, linf;
		_mg->applyBC(0);
		for (int i = 0; i < 12; i++) {
			_mg->relax(0, 4, false);
			_mg->relax(0, 4, true);
		}
		_mg->computeResidual(0, linf, l2);
		cout << "Linf: " << linf << " L2: " << l2 << endl;*/
	}
	else if (key == GLFW_KEY_C)
	{
		//_mg->clearZero(0);
	}
	else
	{
		return false;
	}
	return true;
}

int main() 
{
	cout << "Partikel " << git_version_short << endl;

	auto window = make_unique<GLWindow>("Partikel", 130*8, 130*8);
	window->keyHandlers.push_back(keyHandler);
	CLQueue cpuQueue, gpuQueue;
	createQueues(cpuQueue, gpuQueue);
	auto& queue = gpuQueue;

	auto vaoGrid = make_unique<SingleVertexArray<cl_float4> >();
	vaoGrid->defineAttrib(0, GL_FLOAT, 4, 0);
    //vao->defineAttrib(1, GL_FLOAT, 2, offsetof(VertexStruct, pos));
    
	// grid
	Vec2i gridSize(1024, 1024);
	Vec2i bcGridSize = gridSize + Vec2i(2);
	vector<cl_float4> data(bcGridSize.x * bcGridSize.y);
	vaoGrid->buffer.setData(&data[0], data.size());
	
	MultigridPoisson mgd(gridSize, 25.0f/gridSize.x);
	_mg = &mgd;
	Grid1f grid(gridSize, 1, queue);
	grid.allocHost();

	CLKernel kernel(queue, "display.cl", "display");
	auto clBuf = make_unique<CLVertexBuffer<cl_float4> >(queue, *vaoGrid);
	kernel.setArg(0, grid.cl.handle);
	kernel.setArg(1, clBuf->handle);
	
	auto program = make_unique<ShaderProgram>(
		make_shared<VertexShader>("triangle_test.vs"),
		make_shared<FragmentShader>("triangle_test.fs"),
		make_shared<GeometryShader>("triangle_test.gs") );
	
	Vec2 screen(1920, 1080);

	program->use();
	vaoGrid->bind();

	auto tex = make_unique<Texture>("star02.png");

	float time = 1;
	while(window->poll()) 
	{
		glFinish();
		clBuf->acquire();
		
		time += 0.1f;
		if (time > 1) {
			time--;

			//cout << "Relax" << endl;
			//mgd.vcycle();
		}

		Vec2i sz = mgd.levels[displayLevel]->dim + Vec2i(2);
		kernel.setArg(2, sz);
		float *fd = &mgd.levels[displayLevel]->u[0];
		if (displayGrid == 1)
			fd = &mgd.levels[displayLevel]->b[0];
		if (displayGrid == 2)
			fd = &mgd.levels[displayLevel]->r[0];
		for (int i = 0; i < sz.x*sz.y; i++)
			grid.host[i] = fd[i];
		
		grid.upload();
		kernel.enqueue(sz.x * sz.y);

		clBuf->release();
		clFinish(queue.handle);

		vaoGrid->bind();
		program->setUniform(program->uniform("size"), sz);
		program->setUniform(program->uniform("scale"), Vec2(2.0f / sz.x, 2.0f / sz.y));

		window->clearBuffer();
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDrawArrays(GL_POINTS, 0, (GLsizei)sz.x * sz.y);
		window->swap();
	}

	return 0;	
}