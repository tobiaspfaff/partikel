#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable

struct Vertex { 
	float4 color;
	float2 pos;
};

static float4 hsv2rgb(float4 hsv)
{
    float4 rgb = (float4)0;
    if (hsv.z != 0) 
    {
    	float hh = (hsv.x - floor(hsv.x)) * 6;    	
    	int i = (int) hh;
    	float ff = hh - i;
    	float p = hsv.z * (1.0 - hsv.y);
		float q = hsv.z * (1.0 - hsv.y * ff);
		float t = hsv.z * (1.0 - hsv.y * (1-ff));
		switch(i)
		{
			case 0: rgb = (float4)(hsv.z, t, p, hsv.w); break;
			case 1: rgb = (float4)(q, hsv.z, p, hsv.w); break;
			case 2: rgb = (float4)(p, hsv.z, t, hsv.w); break;
			case 3: rgb = (float4)(p, q, hsv.z, hsv.w); break;
			case 4: rgb = (float4)(t, p, hsv.z, hsv.w); break;
			default: rgb = (float4)(hsv.z, p, q, hsv.w); break;
		}
	}
    return rgb;
}

__kernel void hello(__global struct Vertex* out, const float time, const float2 screen, const int N) {
	size_t tid = get_global_id(0);
	const float freq_time = 0.02;
	const float freq_part = 0.01;
	
	const float phi = freq_time * time + tid * freq_part;		
	const float mod = fmod(tid+time, 1000) / 1000;
	const float R = sin(mod * M_PI * 2);
	const float tx = (sin(phi) * R + 1) * 0.5 * screen.x;
	const float ty = (cos(phi) * R + 1) * 0.5 * screen.y;
	out[tid].pos = (float2)(tx,ty);
   	out[tid].color = hsv2rgb( (float4) (fmod(mod*3,1), 1, 1, (float)tid/(float)N) );   	
}