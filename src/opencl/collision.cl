#include "particle.h"

__kernel void collide(
	__global float2* q,
	__global float2* d,
	__global uint* cellStart, __global uint* cellEnd,
	__global float* weight, __global uint* count,
	float radius, struct Domain domain, uint num)
{
	uint tid = get_global_id(0);
	if (tid >= num)
		return;

	uint cnt = count[tid];
	float2 delta = d[tid];
	float2 pos1 = q[tid];
	int2 cell = getGridPos(pos1, domain);

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

			float2 pos2 = q[j];
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
	d[tid] = delta;
	count[tid] = cnt;
}

__kernel void wallCollideAndApply(
	__global float2* q,
	__global float2* d,
	__global float2* v,
	__global uint* count,
	float invdt,
	float R, struct Domain domain, float omega, uint num)
{
	uint tid = get_global_id(0);
	if (tid >= num)
		return;

	float2 p0 = q[tid];
	float2 pos = p0 - domain.offset - R;
	float2 delta = d[tid];
	int cnt = count[tid];
	float2 size = convert_float2(domain.size) * domain.dx - 2 * R;
	
	// wall collision
	if (pos.x < 0) {
		delta.x -= pos.x;
		cnt++;
	}
	else if (pos.x > size.x) {
		delta.x += size.x - pos.x;
		cnt++;
	}
	if (pos.y < 0) {
		delta.y -= pos.y;
		cnt++;
	}
	else if (pos.y > size.y) {
		delta.y += size.y - pos.y;
		cnt++;
	}

	// apply SOR Jacobi
	float corr = (cnt == 0) ? 0.0f : (omega / (float)cnt);
	q[tid] = p0 + delta * corr;
	v[tid] += (invdt * corr) * delta;
}
