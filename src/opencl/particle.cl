#include "particle.h"

// Basic particle dynamics

__kernel void predictPosition(
	__global float* px, __global float* py, 
	__global float* qx, __global float* qy, // in: velocity, out: xstar
	float dt, uint num)
{
	size_t tid = get_global_id(0);
	if (tid >= num)
		return;
	
	float vx = qx[tid], vy = qy[tid];
	float x = px[tid], y = py[tid];

	// apply gravity
	vy += dt * -9.81;

	// predict x star
	qx[tid] = x + dt * vx;
	qy[tid] = y + dt * vy;
}

__kernel void finalAdvect(
	__global float* px, __global float* py, // in: pos
	__global float* qx, __global float* qy, // in: xstar
	__global float* npx, __global float* npy, // out: pos
	__global float* nvx, __global float* nvy, // out: velocity
	float inv_dt, uint num)
{
	size_t tid = get_global_id(0);
	if (tid >= num)
		return;

	float xs = qx[tid], ys = qy[tid];
	float x = px[tid], y = py[tid];
	
	// update velocity
	float vx = inv_dt * (xs - x);
	float vy = inv_dt * (ys - y);
	//vx *= 0.9f;
	//vy *= 0.9f;
	
	// set new position and velocity
	npx[tid] = xs;
	npy[tid] = ys;
	nvx[tid] = vx;
	nvy[tid] = vy;
}

__kernel void prepareList(
	__global float* px, __global float* py,
	__global uint* hash, __global uint* part, 
	struct Domain domain, uint num)
{
	size_t tid = get_global_id(0);
	if (tid >= num)
		return;

	int2 pos = getGridPos(px[tid], py[tid], domain);
	hash[tid] = getGridHash(pos, domain.size);
	part[tid] = tid;
}

__kernel void calcCellBoundsAndReorder(
	__global uint* hash, __global uint* partIndex,
	__global uint* cellStart, __global uint* cellEnd,
	__local uint* localHash, uint num,
	__global float* px, __global float* py, 
	__global float* qx, __global float* qy,
	__global float* im, __global uint* ph,
	__global float* px2, __global float* py2,
	__global float* qx2, __global float* qy2,
	__global float* im2, __global uint* ph2)
{
	const uint tid = get_global_id(0);
	const uint loc = get_local_id(0);
	uint hash0;

	if (tid < num)
	{
		hash0 = hash[tid];

		//Load hash data into local memory so that we can look 
		//at neighboring particle's hash value without loading
		//two hash values per thread
		localHash[loc + 1] = hash0;

		//First thread in block must load neighbor particle hash
		if (tid > 0 && loc == 0)
			localHash[0] = hash[tid - 1];
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
		uint sortedIndex = partIndex[tid];

		px2[tid] = px[sortedIndex];
		py2[tid] = py[sortedIndex];
		qx2[tid] = qx[sortedIndex];
		qy2[tid] = qy[sortedIndex];
		im2[tid] = im[sortedIndex];
		ph2[tid] = ph[sortedIndex];
	}
}
