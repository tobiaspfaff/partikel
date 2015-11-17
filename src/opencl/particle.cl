// Basic particle dynamics

__kernel void predictPosition(
	__global float* px, __global float* py, 
	__global float* vx, __global float* vy,
	float dt, uint num)
{
	size_t tid = get_global_id(0);
	if (tid >= num)
		return;
	
	// apply gravity
	vy[tid] += dt * -9.81;

	// predict x star
	px[tid] += dt * vx[tid];
	py[tid] += dt * vy[tid];
}

__kernel void finalAdvect(
	__global float* px, __global float* py,
	__global float* dx, __global float* dy,
	__global float* vx, __global float* vy,
	float inv_dt, uint num)
{
	size_t tid = get_global_id(0);
	if (tid >= num)
		return;

	// set velocity
	vx[tid] += inv_dt * dx[tid];
	vy[tid] += inv_dt * dy[tid];
	
	// set position
	px[tid] += dx[tid];
	py[tid] += dy[tid];
}

__kernel void prepareList(
	__global float* px, __global float* py,
	__global uint* hash, __global uint* part, 
	uint grid_x, uint grid_y, uint num)
{
	size_t tid = get_global_id(0);
	if (tid >= num)
		return;

	part[tid] = tid;
	int x = clamp((int)px[tid], 0, (int)grid_x - 1);
	int y = clamp((int)py[tid], 0, (int)grid_y - 1);
	hash[tid] = ((uint)x) + ((uint)y) * grid_x
		;
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

/*


// additional : sort pos, vel
const float R = 0.5f;
for (int i = 0; i <part->size; i++) {
	Vec2 p(px[i], py[i]);
	Vec2i cell(px[i], py[i]);
	for (int dj = -1; dj <= 1; dj++) {
		for (int di = -1; di <= 1; di++)
		{
			Vec2i pj(cell.x + di, cell.y + dj);
			if (pj.x < 0 || pj.y < 0 || pj.x >= grid.x || pj.y >= grid.y)
				continue;
			int hash = pj.x + pj.y * grid.x;
			int start = acellStart[hash], end = acellEnd[hash];
			if (start < 0 || end < 0)
				continue;
			for (int s = start; s < end; s++) {
				int p2 = pairs[s].particle;
				if (p2 == i)
					continue;

				const Vec2 x1(px[i], py[i]);
				const Vec2 x2(px[p2], py[p2]);
				float C = norm(x1 - x2) - 2 * R;
				if (C > 0)
					continue;
				float iw = 1.0f / (part->invmass.buffer[i] + part->invmass.buffer[p2]);
				Vec2 dx1 = (-iw * part->invmass.buffer[i] * C) * normalize(x1 - x2);
				Vec2 dx2 = (-iw * part->invmass.buffer[p2] * C) * -normalize(x1 - x2);
				px[i] += dx1.x;
				py[i] += dx1.y;
				px[p2] += dx2.x;
				py[p2] += dx2.y;
			}
		}
	}
}
*/