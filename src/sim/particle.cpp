#include "sim/particle.hpp"
#include <cassert>
#include <random>

using namespace std;

DynamicParticles::DynamicParticles(int reserve, BufferType type, CLQueue& queue) :
	ParticleBase(reserve, type),
	px(queue, reserve, type),
	py(queue, reserve, type),
	qx(queue, reserve, type),
	qy(queue, reserve, type),
	invmass(queue, reserve, type),
	phase(queue, reserve, type)
{
}

void DynamicParticles::upload()
{
	px.upload();
	py.upload();
	qx.upload();
	qy.upload();
	invmass.upload();
	phase.upload();
}

void DynamicParticles::download()
{
	px.download();
	py.download();
	qx.download();
	qy.download();
	invmass.download();
	phase.download();
}

void DynamicParticles::setSize(int nsize)
{
	size = nsize;
	if (size > reserve)
	{
		px.setSize(nsize);
		py.setSize(nsize);
		qx.setSize(nsize);
		qy.setSize(nsize);
		invmass.setSize(nsize);
		phase.setSize(nsize);
		reserve = nsize;
	}
}

void seedRandom(DynamicParticles& parts, const Vec2& domain, float density)
{
	random_device rd;
	mt19937 gen(rd());
	uniform_real_distribution<float> uniformX(0, domain.x);
	uniform_real_distribution<float> uniformY(0, domain.y);

	int N = (int)(domain.x * domain.y * density);
	parts.setSize(N);

	for (int i = 0; i < N; i++)
	{
		Vec2i p(i % (int)domain.x, (int)(domain.y - 1) - i / (int)domain.x);
		Vec2 pos(uniformX(gen), uniformY(gen));
		//Vec2 pos((float)p.x + 0.5f, (float)p.y + 0.5f);
		parts.px.buffer[i] = pos.x;
		parts.py.buffer[i] = pos.y;
		parts.qx.buffer[i] = 0;
		parts.qy.buffer[i] = 0;
		parts.invmass.buffer[i] = 1.0f;
		parts.phase.buffer[i] = 0;
	}
	parts.upload();
}