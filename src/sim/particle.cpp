#include "sim/particle.hpp"
#include <cassert>
#include <random>

using namespace std;

DynamicParticles::DynamicParticles(int reserve, BufferType type, CLQueue& queue) :
	ParticleBase(reserve, type),
	p(queue, reserve, type),
	q(queue, reserve, type),
	v(queue, reserve, type),
	invmass(queue, reserve, type),
	phase(queue, reserve, type)
{
}

void DynamicParticles::upload()
{
	p.upload();
	q.upload();
	v.upload();
	invmass.upload();
	phase.upload();
}

void DynamicParticles::download()
{
	p.download();
	q.download();
	v.download();
	invmass.download();
	phase.download();
}

void DynamicParticles::setSize(int nsize)
{
	size = nsize;
	if (size > reserve)
	{
		p.resize(nsize);
		q.resize(nsize);
		v.resize(nsize);
		invmass.resize(nsize);
		phase.resize(nsize);
		reserve = nsize;
	}
}

void seedRandom(DynamicParticles& parts, const Domain& domain, float density, float mass)
{
	random_device rd;
	mt19937 gen(rd());
	gen.seed(12);
	uniform_real_distribution<float> uniformX(domain.offset.x, domain.offset.x + domain.dx * domain.size.x);
	uniform_real_distribution<float> uniformY(domain.offset.y, domain.offset.y + domain.dx * domain.size.y);

	int N = (int)(domain.size.x * domain.size.y * density);
	parts.setSize(N);

	for (int i = 0; i < N; i++)
	{
		//Vec2i p(i % (int)domain.x, (int)(domain.y - 1) - i / (int)domain.x);
		Vec2 pos(uniformX(gen), uniformY(gen));
		//Vec2 pos((float)p.x + 0.5f, (float)p.y + 0.5f);
		parts.p.buffer[i] = { pos.x, pos.y };
		parts.q.buffer[i] = { 0, 0 };
		parts.v.buffer[i] = { 0, 0 };
		parts.invmass.buffer[i] = 1.0f/mass;
		parts.phase.buffer[i] = 0;
	}
	parts.upload();
}