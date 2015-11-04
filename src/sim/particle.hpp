// PBD

#ifndef SIM_PARTICLE_HPP
#define SIM_PARTICLE_HPP

#include "compute/computeMain.hpp"
#include "tools/vectors.hpp"

class ParticleBase {
public:
	ParticleBase(int reserve, BufferType type) {}
	virtual ~ParticleBase() {}

	int reserve;
	int size;
};

class DynamicParticles : public ParticleBase {
public:
	DynamicParticles(int reserve, BufferType type, CLQueue& queue);
	void upload();
	void download();
	
	CLBuffer<cl_float> px, py, invmass;
	CLBuffer<cl_int> phase;
};

void seedRandom(DynamicParticles& parts, Vec2& domain, float density);

#endif