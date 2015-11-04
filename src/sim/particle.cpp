#include "sim/particle.hpp"
#include <cassert>

using namespace std;

DynamicParticles::DynamicParticles(int reserve, BufferType type, CLQueue& queue) :
	ParticleBase(reserve, type),
	px(queue, reserve, type),
	py(queue, reserve, type),
	invmass(queue, reserve, type),
	phase(queue, reserve, type)
{
}

void DynamicParticles::upload()
{
	px.upload();
	py.upload();
	invmass.upload();
	phase.upload();
}

void DynamicParticles::download()
{
	px.download();
	py.download();
	invmass.download();
	phase.download();
}
