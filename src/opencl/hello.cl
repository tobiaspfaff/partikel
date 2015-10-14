#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable

struct Vertex { 
	float4 color;
	float2 pos;
};

__kernel void hello(__global struct Vertex* out, const float time, const float2 screen) {
	size_t tid = get_global_id(0);
	const float freq_time = 0.02;
	const float freq_part = 0.01;
	
	const float phi = freq_time * time + tid * freq_part;			
	const float mod = 1;//fmod(tid+time,1000.0) / 1000.0f;
	const float R = 1;//sin(mod * M_PI * 2);
	const float tx = (sin(phi) * R + 1) * 0.5 * screen.x;
	const float ty = (cos(phi) * R + 1) * 0.5 * screen.y;
	out[tid].pos = float2(100, 0);
   	out[tid].color = float4(0,0,0,0);
   	printf("%f %f\n",tx,ty);
}