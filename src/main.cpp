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
	auto partAlt = make_unique<DynamicParticles>(1024, BufferType::Both, queue);
	seedRandom(*part, domain, 1.0f);
	seedRandom(*partAlt, domain, 1.0f);

	// Display handler
	DisplayParticle display(queue, Vec2(0), domain, *window);
	display.attach(part.get(), "P0");

	const float dt = 0.01f;
	Vec2i grid((int)domain.x, (int)domain.y);
	const int wgSize = 32;
	
	cl_uint2 gridSize = {grid.x, grid.y};
	CLBuffer<cl_uint> hash(queue, part->size, BufferType::Both);
	CLBuffer<cl_uint> particle(queue, part->size, BufferType::Both);
	CLBuffer<cl_uint> hash2(queue, part->size, BufferType::Both);
	CLBuffer<cl_uint> particle2(queue, part->size, BufferType::Both);
	CLBuffer<cl_uint> cellStart(queue, part->size, BufferType::Both);
	CLBuffer<cl_uint> cellEnd(queue, part->size, BufferType::Both);
	
	// Kernels
	CLKernel clPredict(queue, "particle.cl", "predictPosition");
	CLKernel clAdvect(queue, "particle.cl", "finalAdvect");	
	CLKernel clPrepareList(queue, "particle.cl", "prepareList");		
	CLKernel clCalcCellBounds(queue, "particle.cl", "calcCellBoundsAndReorder");	
	BitonicSort sorter(queue);

	auto tex = make_unique<Texture>("circle.png");
	tex->bind();

	vector<float> tx(part->size);
	vector<float> ty(part->size);
	vector<int> tidx(part->size);

	while (window->poll())
	{
		glFinish();
		clPredict.call(part->size, 1, part->px, part->py, part->qx, part->qy, (float)dt, part->size);			
		clEnqueueBarrier(queue.handle);
		clPrepareList.call(part->size, 1, part->qx, part->qy, hash, particle, gridSize, part->size);
		clEnqueueBarrier(queue.handle);
		sorter.sort(hash, hash2, particle, particle2);
		clEnqueueBarrier(queue.handle);
		cellStart.fill(0xFFFFFFFFU);
		LocalBlock local((wgSize + 1) * sizeof(cl_uint));
		clCalcCellBounds.call(part->size, wgSize, hash2, particle2, cellStart, cellEnd, local, part->size,
			part->px, part->py, part->qx, part->qy, part->invmass, part->phase,
			partAlt->px, partAlt->py, partAlt->qx, partAlt->qy, partAlt->invmass, partAlt->phase);
		clEnqueueBarrier(queue.handle);
		clFinish(queue.handle);

		part->download();
		partAlt->download();
		particle2.download();
		hash2.download();
		particle.download();
		hash.download();
		cellStart.download();
		cellEnd.download();
		vector<float>& qx = partAlt->qx.buffer;
		vector<float>& qy = partAlt->qy.buffer;
		vector<float>& weight = partAlt->invmass.buffer;

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
		
		for (int iter = 0; iter < 10; iter++)
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
			
			for (int i = 0; i < part->size; i++) {
				tx[i] = ty[i] = 0;
				tidx[i] = 0;
			}
			
			for (int i = 0; i < part->size; i++) {
				Vec2 p(qx[i], qy[i]);
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
						//cout << i << ": " << hash << " -> " << start << "-" << end << endl;

						for (int s = start; s < end; s++) {
							int i2 = s;// particle2.buffer[s];
							if (i2 == i)
								continue;

							Vec2 x1(qx[i], qy[i]);
							Vec2 x2(qx[i2], qy[i2]);
							float C = norm(x1 - x2) - 2 * R;

							if (C > 0)
								continue;
							float iw = 1.0f / (weight[i] + weight[i2]);
							Vec2 dx1 = (-iw * weight[i] * C) * normalize(x1 - x2);
							Vec2 dx2 = (-iw * weight[i2] * C) * -normalize(x1 - x2);
							tx[i] += dx1.x;
							ty[i] += dx1.y;
							tidx[i]++;
							tx[i2] += dx2.x;
							ty[i2] += dx2.y;
							tidx[i2]++;
						}
					}
				}
			}

			for (int i = 0; i < part->size; i++) {
				if (tidx[i] > 0) {
					qx[i] += tx[i] * 1.8f / tidx[i];
					qy[i] += ty[i] * 1.8f / tidx[i];
				}
			}
			
			for (int i = 0; i < domain.x * domain.y; i++) {
				Vec2 p(qx[i], qy[i]);
				p.x = max(min(p.x, domain.x-0.5f), 0.5f);
				p.y = max(min(p.y, domain.y-0.5f), 0.5f);
				qx[i] = p.x;
				qy[i] = p.y;
			}
		}
		part->px.buffer = partAlt->px.buffer;
		part->py.buffer = partAlt->py.buffer;
		part->qx.buffer = partAlt->qx.buffer;
		part->qy.buffer = partAlt->qy.buffer;
		part->px.upload();
		part->py.upload();
		part->qx.upload();
		part->qy.upload();

		clEnqueueBarrier(queue.handle);
		clAdvect.call(part->size, 1, part->px, part->py, part->qx, part->qy, 1.0f/dt, part->size);
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