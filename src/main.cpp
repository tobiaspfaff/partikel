#include <iostream>
#include <memory>
#include <sstream>
#include <vector>
#include <algorithm>
#include "render/opengl.hpp"
#include "render/colors.hpp"
#include "render/window.hpp"
#include "render/displayGrid.hpp"
#include "tools/vectors.hpp"
#include "tools/log.hpp"
#include "compute/computeMain.hpp"
#include "sim/grid.hpp"
#include "sim/mgsolve.hpp"
#include "sim/pressure.hpp"
#include "render/shader.hpp"
#include "render/texture.hpp"
#include "render/vertexArray.hpp"
#include <GLFW/glfw3.h>

using namespace std;

PressureSolver* ps = nullptr;
extern const char* git_version_short;

bool keyHandler(int key) 
{
	switch (key)
	{
	case GLFW_KEY_R:
		ps->solver.relax(0, 100, false);
		return true;
	case GLFW_KEY_F:
		float dummy;
		ps->solver.solve(dummy, 0);
		return true;
	case GLFW_KEY_V:
		ps->solver.vcycle(0);
		return true;
	case GLFW_KEY_C:
		ps->solver.clearZero(0);
		return true;
	default:
		return false;
	}
}

int main() 
{	
	// Init Window
	cout << "Partikel " << git_version_short << endl;
	auto window = make_unique<GLWindow>("Partikel", 100 * 8, 100 * 8);
	window->keyHandlers.push_back(&keyHandler);
	
	// Init CL
	CLQueue cpuQueue, gpuQueue;
	createQueues(cpuQueue, gpuQueue);
	auto& queue = gpuQueue;

	// Grids
	Vec2i gridSize(1024, 1024);
	float h0 = 25.0f / gridSize.x;
	auto vel = make_unique<GridMac2f>(gridSize, 1, BufferType::Both, queue);
	PressureSolver psolve(*vel, h0, queue); 
	ps = &psolve;
	
	// Display handler
	DisplayGrid display(queue, gridSize + Vec2i(4), *window);
	//display.attach(psolve.pressure, "Pressure");
	//display.attach(psolve.divergence, "Divergence");
	for (int i = 0; i < ps->solver.levels.size(); i++)
	{
		stringstream str;
		str << "Level " << i << " ";
		display.attach(&ps->solver.levels[i]->u, str.str() + "U");
		display.attach(&ps->solver.levels[i]->b, str.str() + "B");
		display.attach(&ps->solver.levels[i]->r, str.str() + "R");
	}

	// MG Test
	for (int i = 0; i < gridSize.x; i++)
	{
		for (int j = 0; j < gridSize.y; j++) 
		{
			float* ptr = psolve.divergence->ptr(i, j);
			Vec2 c((float)i / gridSize.x, (float)j / gridSize.y);
			bool ip = c.y > 0.3 && c.y < 0.6 && c.x > 0.2 && c.x < 0.3;
			bool im = c.y > 0.3 && c.y < 0.6 && c.x > 0.5 && c.x < 0.6;
			*ptr = 0;
			if (ip) *ptr = 0.1;
			if (im) *ptr = -0.1;
		}
	}

	while(window->poll()) 
	{
		glFinish();
		display.compute();
		clFinish(queue.handle);

		window->clearBuffer();
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		display.render();
		window->swap();
	}

	return 0;	
}