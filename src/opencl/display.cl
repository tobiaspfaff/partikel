#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable

__kernel void displayGrid(__global float* in, __global float* out) {
	size_t tid = get_global_id(0);
	out[tid] = in[tid];
}