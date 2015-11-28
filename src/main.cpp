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
#include "compute/gpuSort.hpp"
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

struct NPair
{
	int hash;
	int particle;

	bool operator<(const NPair& a) 
	{
		return hash < a.hash;
	}
};

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
	Vec2 domain(256, 256);
	auto part1 = make_unique<DynamicParticles>(1024, BufferType::Both, queue);
	auto part2 = make_unique<DynamicParticles>(1024, BufferType::Both, queue);
	seedRandom(*part1, domain, 1.0f);
	seedRandom(*part2, domain, 1.0f);
	int parts = part1->size;

	// Display handler
	DisplayParticle display(queue, Vec2(0), domain, *window);
	display.attach(part1.get(), "P0");

	float dt = 0.01f;
	Vec2i grid((int)domain.x, (int)domain.y);
	const int wgSize = 32;
	const float R = 0.5f;

	cl_uint2 gridSize = { grid.x, grid.y };
	cl_float2 domainSize = { domain.x, domain.y };

	// Temporary buffers for sorting
	CLBuffer<cl_uint> hash(queue, parts, BufferType::Both);
	CLBuffer<cl_uint> hashSorted(queue, parts, BufferType::Both);
	CLBuffer<cl_uint> particleIndex(queue, parts, BufferType::Both);
	CLBuffer<cl_uint> particleIndexSorted(queue, parts, BufferType::Both);
	
	// Kernels
	CLKernel clPredict(queue, "particle.cl", "predictPosition");
	CLKernel clAdvect(queue, "particle.cl", "finalAdvect");	
	CLKernel clPrepareList(queue, "particle.cl", "prepareList");		
	CLKernel clCalcCellBounds(queue, "particle.cl", "calcCellBoundsAndReorder");
	CLKernel clCollide(queue, "collision.cl", "collide");
	CLKernel clWallCollide(queue, "collision.cl", "wallCollideAndApply");
	BitonicSort sorter(queue);

	auto tex = make_unique<Texture>("circle.png");
	tex->bind();

	while (window->poll())
	{		
		glFinish();
		auto cur = part1.get();
		auto alt = part2.get();

		// Predict particle position, apply gravity
		clPredict.call(parts, 1, cur->px, cur->py, cur->qx, cur->qy, dt, parts);			
		clEnqueueBarrier(queue.handle);

		// Sort particles by cell hash
		clPrepareList.call(parts, 1, cur->qx, cur->qy, hash, particleIndex, gridSize, parts);
		clEnqueueBarrier(queue.handle);
		sorter.sort(hash, hashSorted, particleIndex, particleIndexSorted);
		clEnqueueBarrier(queue.handle);

		// Re-order particles and collect neighborhood information
		auto& cellStart = particleIndex;
		auto& cellEnd = hash;
		cellStart.fill(0xFFFFFFFFU);
		LocalBlock local((wgSize + 1) * sizeof(cl_uint));
		clCalcCellBounds.call(parts, wgSize, hashSorted, particleIndexSorted, cellStart, cellEnd, local, parts,
			cur->px, cur->py, cur->qx, cur->qy, cur->invmass, cur->phase,
			alt->px, alt->py, alt->qx, alt->qy, alt->invmass, alt->phase);
		clEnqueueBarrier(queue.handle);
		swap(cur, alt);

		// iterate on constraints
		for (int iter = 0; iter < 5; iter++)
		{
			auto& deltaX = alt->qx;
			auto& deltaY = alt->qy;
			auto& counter = hashSorted;
			counter.fill(0);
			deltaX.fill(0);
			deltaY.fill(0);
			clEnqueueBarrier(queue.handle);

			// particle - particle collisions
			const float SOR = 1.8f;
			clCollide.call(parts, 1, cur->qx, cur->qy, deltaX, deltaY, cellStart, cellEnd,
				cur->invmass, counter, R, gridSize, parts);
			clEnqueueBarrier(queue.handle);

			// add wall collision constraints, apply SOR Jacobi
			clWallCollide.call(parts, 1, cur->qx, cur->qy, 
				deltaX, deltaY, counter, R, domainSize, SOR, parts);			
			clEnqueueBarrier(queue.handle);
		}
		
		// apply position delta
		clAdvect.call(parts, 1, cur->px, cur->py, cur->qx, cur->qy, 
			alt->px, alt->py, alt->qx, alt->qy, 1.0f/dt, parts);
		clEnqueueBarrier(queue.handle);
		swap(cur, alt);

		// copy particles to display buffer
		assert(cur == part1.get());
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