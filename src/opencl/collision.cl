#include "particle.h"

__kernel void collide(
	__global float* qx, __global float* qy,
	__global float* dx, __global float* dy,
	__global uint* cellStart, __global uint* cellEnd,
	__global float* weight, __global uint* count,
	float radius, struct Domain domain, uint num)
{
	uint tid = get_global_id(0);
	if (tid >= num)
		return;

	uint cnt = count[tid];
	float2 delta = (float2)(dx[tid], dy[tid]);
	float2 pos1  = (float2)(qx[tid], qy[tid]);
	int2 cell = getGridPos(pos1.x, pos1.y, domain);

	float R2 = 4.0 * radius * radius;

	for (int y = -1; y <= 1; y++)
	for (int x = -1; x <= 1; x++) 
	{
		uint hash = getGridHash(cell + (int2)(x,y), domain.size);
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
	float R, struct Domain domain, float omega, uint num)
{
	uint tid = get_global_id(0);
	if (tid >= num)
		return;

	float x0 = qx[tid], y0 = qy[tid];
	float x = x0 - domain.offset.x - R;
	float y = y0 - domain.offset.y - R;
	float tx = dx[tid];
	float ty = dy[tid];
	int cnt = count[tid];
	float sx = domain.size.x * domain.dx - 2 * R;
	float sy = domain.size.y * domain.dx - 2 * R;
	
	// wall collision
	if (x < 0) {
		tx -= x;
		cnt++;
	}
	else if (x > sx) {
		tx += sx - x;
		cnt++;
	}
	if (y < 0) {
		ty -= y;
		cnt++;
	}
	else if (y > sy) {
		ty += sy - y;
		cnt++;
	}

	// apply SOR Jacobi
	float corr = (cnt == 0) ? 0.0f : (omega / (float)cnt);
	qx[tid] = x0 + tx * corr;
	qy[tid] = y0 + ty * corr;
}
