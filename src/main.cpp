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
	Vec2 domain(32, 32);
	auto part = make_unique<DynamicParticles>(1024, BufferType::Both, queue);
	seedRandom(*part, domain, 1.0f);

	// Display handler
	DisplayParticle display(queue, Vec2(0), domain, *window);
	display.attach(part.get(), "P0");

	const float dt = 0.01f;
	Vec2i grid((int)domain.x, (int)domain.y);
	const int wgSize = 32;
	
	CLBuffer<cl_uint> hash(queue, part->size, BufferType::Both);
	CLBuffer<cl_uint> particle(queue, part->size, BufferType::Both);
	CLBuffer<cl_uint> hash2(queue, part->size, BufferType::Both);
	CLBuffer<cl_uint> particle2(queue, part->size, BufferType::Both);
	CLBuffer<cl_uint> cellStart(queue, part->size, BufferType::Both);
	CLBuffer<cl_uint> cellEnd(queue, part->size, BufferType::Both);
	CLBuffer<cl_float> dx(queue, part->size, BufferType::Both);
	CLBuffer<cl_float> dy(queue, part->size, BufferType::Both);

	// Kernels
	CLKernel clPredict(queue, "particle.cl", "predictPosition");
	clPredict.setArgs(part->px.handle, part->py.handle, 
		part->vx.handle, part->vy.handle, (float)dt, part->size);
	
	CLKernel clAdvect(queue, "particle.cl", "finalAdvect");
	clAdvect.setArgs(part->px.handle, part->py.handle, dx.handle,
		dy.handle, part->vx.handle, part->vy.handle, 1.0f / dt, part->size);
	
	CLKernel clPrepareList(queue, "particle.cl", "prepareList");
	clPrepareList.setArgs(part->px.handle, part->py.handle, hash.handle, 
		particle.handle, (unsigned)grid.x, (unsigned)grid.y, part->size);
		
	CLKernel clCalcCellBounds(queue, "particle.cl", "calcCellBounds");
	clCalcCellBounds.setArgs(hash2.handle, particle2.handle, 
		cellStart.handle, cellEnd.handle, LocalBlock((wgSize + 1) * sizeof(cl_uint)), part->size);

	BitonicSort sorter(queue);

	auto tex = make_unique<Texture>("circle.png");
	tex->bind();

	while (window->poll())
	{
		glFinish();
		clPredict.enqueue(part->size);
		clEnqueueBarrier(queue.handle);
		clPrepareList.enqueue(part->size);
		clEnqueueBarrier(queue.handle);
		sorter.sort(hash, hash2, particle, particle2);
		clEnqueueBarrier(queue.handle);
		cellStart.fill(0xFFFFFFFFU);
		dx.fill(0U);
		dy.fill(0U);
		clCalcCellBounds.enqueue(part->size, wgSize);
		clEnqueueBarrier(queue.handle);
		clFinish(queue.handle);

		part->px.download();
		part->py.download();
		dx.download();
		dy.download();
		particle2.download();
		hash2.download();
		particle.download();
		hash.download();
		cellStart.download();
		cellEnd.download();

		/*vector<float>& px = part->qx.buffer;
		vector<float>& py = part->qy.buffer;
		int N = grid.x * grid.y;

		vector<NPair> pairs(part->size);
		vector<int> acellStart(N, -1);
		vector<int> acellEnd(N, -1);
		for (int i = 0; i < pairs.size(); i++)
		{
			//Vec2 p(px[i], py[i]);
			//Vec2i cell(px[i], py[i]);
			//pairs[i].hash = cell.x + grid.x * cell.y;
			pairs[i].particle = particle2.buffer[i];
			pairs[i].hash = hash2.buffer[i];
		}
		//		sort(pairs.begin(), pairs.end());
		
		for (int i = 0; i < pairs.size(); i++)
		{
			int part = pairs[i].particle;
			int hash = pairs[i].hash;
			if (i == 0 || hash != pairs[i - 1].hash)
			{
				acellStart[hash] = i;
				if (i > 0)
					acellEnd[pairs[i - 1].hash] = i;
			}
			if (i == pairs.size() - 1)
				acellEnd[hash] = i + 1;
		}
		*/
		// additional : sort pos, vel
		const float R = 0.5f;
		for (int i = 0; i <part->size; i++) {
			Vec2 p(part->px.buffer[i] + dx.buffer[i], part->py.buffer[i] + dy.buffer[i]);
			Vec2i cell((int)p.x, (int)p.y);
			for (int dj = -1; dj <= 1; dj++) {
				for (int di = -1; di <= 1; di++)
				{
					Vec2i pj(cell.x + di, cell.y + dj);
					if (pj.x < 0 || pj.y < 0 || pj.x >= grid.x || pj.y >= grid.y)
						continue;
					int hash = pj.x + pj.y * grid.x;
					int start = cellStart.buffer[hash], end = cellEnd.buffer[hash];
					if (start < 0 || end < 0)
						continue;
					for (int s = start; s < end; s++) {
						int i2 = particle2.buffer[s];
						if (i2 == i)
							continue;
						
						Vec2 x1(part->px.buffer[i] + dx.buffer[i], part->py.buffer[i] + dy.buffer[i]);
						Vec2 x2(part->px.buffer[i2] + dx.buffer[i2], part->py.buffer[i2] + dy.buffer[i2]);
						float C = norm(x1 - x2) - 2 * R;
						if (C > 0)
							continue;
						float iw = 1.0f / (part->invmass.buffer[i] + part->invmass.buffer[i2]);
						Vec2 dx1 = (-iw * part->invmass.buffer[i] * C) * normalize(x1 - x2);
						Vec2 dx2 = (-iw * part->invmass.buffer[i2] * C) * -normalize(x1 - x2);
						dx.buffer[i] += dx1.x;
						dy.buffer[i] += dx1.y;
						dx.buffer[i2] += dx2.x;
						dy.buffer[i2] += dx2.y;
					}
				}
			}
		}
		
		for (int iter = 0; iter < 1; iter++)
		{
			/*vector<float> de(100 * 100, 0);
			vector<int> cnt(100 * 100, 0);

			for (int i = 0; i < 100*99; i++) {
				float& x1 = part->qy.buffer[i];
				float& x2 = part->qy.buffer[i+100];
				float d = 1.2f;
				float delta = 0.5 * ((x1 - x2) - d);
				if (delta < 0) delta = 0;
				if (i >= 100) {
					de[i] -= delta;
					cnt[i]++;
				}
				de[i+100] += delta;
				cnt[i+100]++;
			}
			for (int i = 0; i < 100 * 100; i++) {
				if (cnt[i] > 0) {
					part->qy.buffer[i] += 1.5f * de[i] / cnt[i];
				}
			}*/

			for (int i = 0; i < domain.x * domain.y; i++) {
				Vec2 p(part->px.buffer[i] + dx.buffer[i], part->py.buffer[i] + dy.buffer[i]);
				p.x = max(min(p.x, domain.x-0.5f), 0.5f);
				p.y = max(min(p.y, domain.y-0.5f), 0.5f);
				dx.buffer[i] = p.x - part->px.buffer[i];
				dy.buffer[i] = p.y - part->py.buffer[i];
			}
		}
		dx.upload();
		dy.upload();

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