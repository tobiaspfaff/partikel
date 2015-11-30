#include "particle.h"

// Basic particle dynamics

__kernel void predictPosition(
	__global float2* p, __global float2* q, 
	__global float2* v,
	float dt, uint num)
{
	size_t tid = get_global_id(0);
	if (tid >= num)
		return;
	
	float2 vel = v[tid];
	float2 pos = p[tid];

	// apply gravity
	vel.y += dt * -9.81;

	// predict x star
	q[tid] = pos + dt * vel;
	v[tid] = vel;
}

__kernel void finalAdvect(
	__global float2* q, // in: pos
	//__global float2* q, // in: xstar
	__global float2* v, // in: velocity
	__global float2* np, // out: pos
	__global float2* nv, // out: velocity
	float inv_dt, uint num)
{
	size_t tid = get_global_id(0);
	if (tid >= num)
		return;

	float2 xstar = q[tid];
	//float2 pos = p[tid];
	
	// update velocity
	//float2 vel = inv_dt * (xstar - pos);
	//vx *= 0.9f;
	//vy *= 0.9f;
	
	// set new position and velocity
	np[tid] = xstar;
	nv[tid] = v[tid];
}

__kernel void prepareList(
	__global float2* p,
	__global uint2* sortArray,
	struct Domain domain, uint num)
{
	size_t tid = get_global_id(0);
	if (tid >= num)
		return;

	int2 pos = getGridPos(p[tid], domain);
	sortArray[tid] = (uint2)(getGridHash(pos, domain.size), tid);
}

__kernel void calcCellBoundsAndReorder(
	__global uint2* sortArray,
	__global uint* cellStart, __global uint* cellEnd,
	__local uint* localHash, uint num,
	__global float2* p,
	__global float2* q,
	__global float2* v,
	__global float* im, __global uint* ph,
	__global float2* p2,
	__global float2* q2,
	__global float2* v2,
	__global float* im2, __global uint* ph2)
{
	const uint tid = get_global_id(0);
	const uint loc = get_local_id(0);
	uint hash0;

	if (tid < num)
	{
		hash0 = sortArray[tid].x;

		//Load hash data into local memory so that we can look 
		//at neighboring particle's hash value without loading
		//two hash values per thread
		localHash[loc + 1] = hash0;

		//First thread in block must load neighbor particle hash
		if (tid > 0 && loc == 0)
			localHash[0] = sortArray[tid - 1].x;
	}

	barrier(CLK_LOCAL_MEM_FENCE);

	if (tid < num){
		if (tid == 0) //Border case
		{
			cellStart[hash0] = 0;
		}
		else //Main case
		{
			if (hash0 != localHash[loc])
				cellEnd[localHash[loc]] = cellStart[hash0] = tid;
		};

		//Another border case
		if (tid == num - 1)
			cellEnd[hash0] = num;

		//Now use the sorted index to reorder the pos and vel arrays
		uint sortedIndex = sortArray[tid].y;

		p2[tid] = p[sortedIndex];
		q2[tid] = q[sortedIndex];
		v2[tid] = v[sortedIndex];
		im2[tid] = im[sortedIndex];
		ph2[tid] = ph[sortedIndex];
	}
}
