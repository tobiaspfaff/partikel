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
#include "sim/semilagrange.hpp"
#include "sim/mgsolve.hpp"
#include "sim/pressure.hpp"
#include "render/shader.hpp"
#include "render/texture.hpp"
#include "render/vertexArray.hpp"
#include <GLFW/glfw3.h>

using namespace std;

/*GridMac2f* tempGrid;
GridMac2f* velGrid;
PressureSolver* ps = nullptr;
float h0;*/

extern const char* git_version_short;

bool keyHandler(int key, int mods) 
{
	switch (key)
	{
	/*case GLFW_KEY_R:
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
	case GLFW_KEY_P:
		ps->solve();
		return true;
	case GLFW_KEY_A:
		semiLagrangeSelfAdvect(*velGrid, *tempGrid, 1, h0);
		return true;*/
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
	/*Vec2i gridSize(256,256);
	h0 = 25.0f / gridSize.x;
	auto vel = make_unique<GridMac2f>(gridSize, 1, BufferType::Both, queue);
	auto temp = make_unique<GridMac2f>(gridSize, 1, BufferType::Both, queue);
	for (int j = 0; j < gridSize.y; j++)
	{
		for (int i = 0; i < gridSize.x; i++)
		{
			Vec2 pos((float)i / gridSize.x, (float)j / gridSize.y);
			if (pos.x > 0.4 && pos.x < 0.6 && pos.y > 0.2 && pos.y < 0.4)
				*vel->ptrV(i, j) = 0.5;
		}
	}
	PressureSolver psolve(*vel, h0, queue); 
	ps = &psolve;
	velGrid = vel.get();
	tempGrid = temp.get();*/
	
	// Display handler
	//DisplayGrid display(queue, gridSize + Vec2i(4), *window);
	//display.attach(psolve.pressure, "Pressure");
	//display.attach(psolve.divergence, "Divergence");
	/*for (int i = 0; i < ps->solver.levels.size(); i++)
	{
		stringstream str;
		str << "Level " << i << " ";
		display.attach(&ps->solver.levels[i]->u, str.str() + "U");
		display.attach(&ps->solver.levels[i]->b, str.str() + "B");
		display.attach(&ps->solver.levels[i]->r, str.str() + "R");
	}*/
	//display.attach(vel.get(), "Velocity");
	
	// MG Test
	/*for (int i = 0; i < gridSize.x; i++)
	{
		for (int j = 0; j < gridSize.y; j++) 
		{
			float* ptr = psolve.divergence->ptr(i, j);
			Vec2 c((float)i / gridSize.x, (float)j / gridSize.y);
			bool ip = c.y > 0.3 && c.y < 0.6 && c.x > 0.2 && c.x < 0.3;
			bool im = c.y > 0.3 && c.y < 0.6 && c.x > 0.5 && c.x < 0.6;
			*ptr = 0;
			if (ip) *ptr = 0.1f;
			if (im) *ptr = -0.1f;
		}
	}*/

	while(window->poll()) 
	{
		//ps->solve();
		//semiLagrangeSelfAdvect(*velGrid, *tempGrid, 1, h0);

		glFinish();
		//display.compute();
		clFinish(queue.handle);

		window->clearBuffer();
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//display.render();
		window->swap();
	}

	return 0;	
}