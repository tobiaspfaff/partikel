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


int main() 
{	
	// Init Window
	cout << "Partikel " << git_version_short << endl;
	auto window = make_unique<GLWindow>(1000,1000);
	window->keyHandlers.push_back(&keyHandler);
	
	// Init CL
	CLQueue cpuQueue, gpuQueue;
	createQueues(cpuQueue, gpuQueue);
	auto& queue = gpuQueue;

	// Particles
	const float R = 0.01f;
	Domain domain = { { 0, 0 }, {512, 512}, 2*R };
	auto part1 = make_unique<DynamicParticles>(1024, BufferType::Both, queue);
	auto part2 = make_unique<DynamicParticles>(1024, BufferType::Both, queue);
	seedRandom(*part1, domain, 0.5f, 0.01f);
	seedRandom(*part2, domain, 0.5f, 0.01f);
	int parts = part1->size;

	// Display handler
	DisplayParticle display(queue, domain, *window);
	display.attach(part1.get(), "P0");
	display.setRadius(R);

	// Temporary buffers for sorting
	int gridElems = domain.size.x * domain.size.y;
	CLBuffer<cl_uint> cellStart(queue, gridElems, BufferType::Gpu); // hashgrid
	CLBuffer<cl_uint> cellEnd(queue, gridElems, BufferType::Gpu);
	CLBuffer<cl_uint2> sortArray(queue, parts, BufferType::Gpu); // (hash, part.index)
	
	// Kernels
	CLKernel clPredict(queue, "particle.cl", "predictPosition");
	CLKernel clAdvect(queue, "particle.cl", "finalAdvect");	
	CLKernel clPrepareList(queue, "particle.cl", "prepareList");		
	CLKernel clCalcCellBounds(queue, "particle.cl", "calcCellBoundsAndReorder");
	CLKernel clCollide(queue, "collision.cl", "collide");
	CLKernel clWallCollide(queue, "collision.cl", "wallCollideAndApply");
	//BitonicSort sorter(queue);
	RadixSort sorter(queue);

	auto tex = make_unique<Texture>("circle.png");
	tex->bind();

	int numIter = 4;

	float dt = 1.0f / 60.0f * 1.0f;
	const int wgSize = 64;
	const int substeps = 18;
	const int subcols = 1;
	const int citer = 5;

	while (window->poll())
	{		
 		if (numIter-- <= 0 && 0)
			break;
		glFinish();
		auto cur = part1.get();
		auto alt = part2.get();
		float localDt = dt / substeps;

		for (int substep = 0; substep < substeps; substep++) {
			// Predict particle position, apply gravity
			clPredict.call(parts, wgSize, cur->px, cur->py, cur->qx, cur->qy, localDt, parts);
			clEnqueueBarrier(queue.handle);

			for (int subcol = 0; subcol < subcols; subcol++) {
				// Sort particles by cell hash
				clPrepareList.call(parts, wgSize, cur->qx, cur->qy, sortArray, domain, parts);
				clEnqueueBarrier(queue.handle);
				//sorter.sort(hash, hashSorted, particleIndex, particleIndexSorted, parts);
				sorter.sort(sortArray, parts);
				clEnqueueBarrier(queue.handle);

				// Re-order particles and collect neighborhood information
				cellStart.fill(0xFFFFFFFFU);
				LocalBlock local((wgSize + 1) * sizeof(cl_uint));
				clCalcCellBounds.call(parts, wgSize, sortArray, cellStart, cellEnd, local, parts,
					cur->px, cur->py, cur->qx, cur->qy, cur->invmass, cur->phase,
					alt->px, alt->py, alt->qx, alt->qy, alt->invmass, alt->phase);
				clEnqueueBarrier(queue.handle);
				swap(cur, alt);

				// iterate on constraints
				for (int iter = 0; iter < citer; iter++)
				{
					auto& deltaX = alt->qx;
					auto& deltaY = alt->qy;
					auto& counter = alt->phase;
					counter.fill(0);
					deltaX.fill(0);
					deltaY.fill(0);
					clEnqueueBarrier(queue.handle);

					// particle - particle collisions
					const float SOR = 1.8f;
					clCollide.call(parts, wgSize, cur->qx, cur->qy, deltaX, deltaY, cellStart, cellEnd,
						cur->invmass, counter, R, domain, parts);
					clEnqueueBarrier(queue.handle);

					// add wall collision constraints, apply SOR Jacobi
					clWallCollide.call(parts, wgSize, cur->qx, cur->qy,
						deltaX, deltaY, counter, R, domain, SOR, parts);
					clEnqueueBarrier(queue.handle);
				}
			}

			// apply position delta
			clAdvect.call(parts, wgSize, cur->px, cur->py, cur->qx, cur->qy,
				alt->px, alt->py, alt->qx, alt->qy, 1.0f / localDt, parts);
			clEnqueueBarrier(queue.handle);
			swap(cur, alt);			
		}
		// copy particles to display buffer
		assert(cur == part1.get());
		display.compute();
		clFinish(queue.handle);

		part1->download();
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