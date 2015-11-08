// Basic particle dynamics

__kernel void predictPosition(
	__global float* px, __global float* py, 
	__global float* qx, __global float* qy,
	__global float* vx, __global float* vy,
	float dt)
{
	size_t tid = get_global_id(0);
	
	// apply gravity
	vy[tid] += dt * -9.81;

	// predict x star
	qx[tid] = px[tid] + dt * vx[tid];
	qy[tid] = py[tid] + dt * vy[tid];
}

__kernel void finalAdvect(
	__global float* px, __global float* py,
	__global float* qx, __global float* qy,
	__global float* vx, __global float* vy,
	float inv_dt)
{
	size_t tid = get_global_id(0);

	// set velocity
	vx[tid] = inv_dt * (qx[tid] - px[tid]);
	vy[tid] = inv_dt * (qy[tid] - py[tid]);
	
	// set position
	px[tid] = qx[tid];
	py[tid] = qy[tid];
}

