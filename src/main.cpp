#include <iostream>
#include <memory>
#include <sstream>
#include <vector>
#include <algorithm>
#include "render/opengl.hpp"
#include "render/colors.hpp"
#include "render/window.hpp"
#include "render/displayParticle.hpp"
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
#include <chrono>
#include <thread>

using namespace std;

extern const char* git_version_short;

bool keyHandler(int key, int mods) 
{
	switch (key)
	{	
	default:
		return false;
	}
}

int main() 
{	
	// Init Window
	cout << "Partikel " << git_version_short << endl;
	auto window = make_unique<GLWindow>(100 * 8, 100 * 8);
	window->keyHandlers.push_back(&keyHandler);
	
	// Init CL
	CLQueue cpuQueue, gpuQueue;
	createQueues(cpuQueue, gpuQueue);
	auto& queue = gpuQueue;

	// Particles
	Vec2 domain(100, 100);
	auto part = make_unique<DynamicParticles>(1024, BufferType::Both, queue);
	seedRandom(*part, domain, 1.0f);

	// Display handler
	DisplayParticle display(queue, Vec2(0), domain, *window);
	display.attach(part.get(), "P0");

	const float dt = 0.01f;

	// Kernels
	CLKernel clPredict(queue, "particle.cl", "predictPosition");
	clPredict.setArg(0, part->px.handle);
	clPredict.setArg(1, part->py.handle);
	clPredict.setArg(2, part->qx.handle);
	clPredict.setArg(3, part->qy.handle);
	clPredict.setArg(4, part->vx.handle);
	clPredict.setArg(5, part->vy.handle);
	clPredict.setArg(6, dt);

	CLKernel clAdvect(queue, "particle.cl", "finalAdvect");
	clAdvect.setArg(0, part->px.handle);
	clAdvect.setArg(1, part->py.handle);
	clAdvect.setArg(2, part->qx.handle);
	clAdvect.setArg(3, part->qy.handle);
	clAdvect.setArg(4, part->vx.handle);
	clAdvect.setArg(5, part->vy.handle);
	clAdvect.setArg(6, 1.0f/dt);

	auto tex = make_unique<Texture>("star02.png");
	tex->bind();

	while(window->poll()) 
	{
		glFinish();
		//std::this_thread::sleep_for(std::chrono::milliseconds(10));
		clPredict.enqueue(part->size);
		clEnqueueBarrier(queue.handle);
		clAdvect.enqueue(part->size);
		clEnqueueBarrier(queue.handle);
		display.compute();
		clFinish(queue.handle);

		window->clearBuffer();
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		display.render();
		window->swap();
	}

	return 0;	
}

/* grid stuff
// Grids
Vec2i gridSize(256,256);
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
tempGrid = temp.get();

// Display handler
//DisplayGrid display(queue, gridSize + Vec2i(4), *window);
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
//display.attach(vel.get(), "Velocity");

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
if (ip) *ptr = 0.1f;
if (im) *ptr = -0.1f;
}
}

*/