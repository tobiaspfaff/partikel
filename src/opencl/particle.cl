// Basic particle dynamics

__kernel void predictPosition(
	__global float* px, __global float* py, 
	__global float* qx, __global float* qy,
	__global float* vx, __global float* vy,
	float dt, uint num)
{
	size_t tid = get_global_id(0);
	if (tid >= num)
		return;
	
	// apply gravity
	//vy[tid] += dt * -9.81;

	// predict x star
	qx[tid] = px[tid] + dt * vx[tid];
	qy[tid] = py[tid] + dt * vy[tid];
}

__kernel void finalAdvect(
	__global float* px, __global float* py,
	__global float* qx, __global float* qy,
	__global float* vx, __global float* vy,
	float inv_dt, uint num)
{
	size_t tid = get_global_id(0);
	if (tid >= num)
		return;

	// set velocity
	vx[tid] = inv_dt * (qx[tid] - px[tid]);
	vy[tid] = inv_dt * (qy[tid] - py[tid]);
	
	// set position
	px[tid] = qx[tid];
	py[tid] = qy[tid];
}

__kernel void prepareList(
	__global float* px, __global float* py,
	__global uint* hash, __global uint* part, 
	uint grid_stride, uint num)
{
	size_t tid = get_global_id(0);
	if (tid >= num)
		return;

	part[tid] = tid;
	hash[tid] = ((uint)px[tid]) + ((uint)py[tid]) * grid_stride;
}

__kernel void calcCellBounds(
	__global uint* hash, __global uint* part,
	__global uint* cellStart, __global uint* cellEnd,
	__local uint* localHash, uint num)
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
		/*uint sortedIndex = d_Index[index];
		float4 pos = d_Pos[sortedIndex];
		float4 vel = d_Vel[sortedIndex];

		d_ReorderedPos[index] = pos;
		d_ReorderedVel[index] = vel;*/
	}
}