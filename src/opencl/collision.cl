
inline int2 getGridPos(float2 pos)
{
	return (int2)((int)pos.x, (int)pos.y);
}

inline uint getGridHash(int2 pos, uint2 gridSize)
{
	// wrap addressing
	return ((uint)pos.x & (gridSize.x-1)) + 
		   ((uint)pos.y & (gridSize.y-1)) * gridSize.x;
}

__kernel void collide(
	__global float* qx, __global float* qy,
	__global float* dx, __global float* dy,
	__global uint* cellStart, __global uint* cellEnd,
	__global float* weight, __global uint* count,
	float radius, uint2 gridSize, uint num)
{
	uint tid = get_global_id(0);
	if (tid >= num)
		return;

	uint cnt = count[tid];
	float2 delta = (float2)(dx[tid], dy[tid]);
	float2 pos1  = (float2)(qx[tid], qy[tid]);
	int2 cell = getGridPos(pos1);

	float R2 = 4.0 * radius * radius;

	for (int y = -1; y <= 1; y++)
	for (int x = -1; x <= 1; x++) 
	{
		uint hash = getGridHash(cell + (int2)(x,y), gridSize);
		uint start = cellStart[hash];
		//printf("%d: %d %d : %d - %d\n", tid, x, y, start, cellEnd[hash]);

		// empty
		if (start == 0xFFFFFFFFU)
			continue;

		uint end = cellEnd[hash];
		for (uint j = start; j < end; j++)
		{
			// don't collide with yourself
			if (j == tid)
				continue;

			float2 pos2 = (float2)(qx[j], qy[j]);
			float2 diff = pos1 - pos2;
			float len2 = dot(diff, diff);

			if (len2 > R2)
				continue;
			float C = half_sqrt(len2) - 2.0f * radius;

			float w1 = weight[tid], w2 = weight[j];
			float cn = w1 / (w1 + w2) * C;
			float2 n = fast_normalize(diff);

			delta.x -= cn * n.x;
			delta.y -= cn * n.y;
			cnt++;			
		}
	}
	
	//printf("%d: %f %f %d\n", tid, delta.x, delta.y, cnt);
	dx[tid] = delta.x;
	dy[tid] = delta.y;
	count[tid] = cnt;
}

__kernel void wallCollideAndApply(
	__global float* qx, __global float* qy,
	__global float* dx, __global float* dy, __global uint* count,
	float R, float2 domain, float omega, uint num)
{
	uint tid = get_global_id(0);
	if (tid >= num)
		return;

	float x = qx[tid];
	float y = qy[tid];
	float tx = dx[tid];
	float ty = dy[tid];
	int cnt = count[tid];
	
	// wall collision
	if (x < R) {
		tx += R - x;
		cnt++;
	}
	else if (x > domain.x - R) {
		tx += domain.x - R - x;
		cnt++;
	}
	if (y < R) {
		ty += R - y;
		cnt++;
	}
	else if (y > domain.y - R) {
		ty += domain.y - R - y;
		cnt++;
	}

	// apply SOR Jacobi
	float corr = (cnt == 0) ? 0.0f : (omega / (float)cnt);
	qx[tid] = x + tx * corr;
	qy[tid] = y + ty * corr;	
}
