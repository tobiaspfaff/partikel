// PBD

#ifndef SIM_PARTICLE_HPP
#define SIM_PARTICLE_HPP

#include "compute/computeMain.hpp"
#include "tools/vectors.hpp"

class ParticleBase {
public:
	ParticleBase(int reserve, BufferType type) : reserve(reserve) {}
	virtual ~ParticleBase() {}

	int reserve;
	int size = 0;
};

class DynamicParticles : public ParticleBase {
public:
	DynamicParticles(int reserve, BufferType type, CLQueue& queue);
	void upload();
	void download();
	void setSize(int nsize);
	
	CLBuffer<cl_float> px, py;
	CLBuffer<cl_float> qx, qy;
	CLBuffer<cl_float> invmass;
	CLBuffer<cl_int> phase;
};

void seedRandom(DynamicParticles& parts, const Vec2& domain, float density);

#endif