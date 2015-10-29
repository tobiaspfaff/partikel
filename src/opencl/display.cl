#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable

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

__kernel void display(__global float* in, __global float4* out, const int2 size) {
	size_t tid = get_global_id(0);
	//int px = tid / size.x;
	//int py = tid - px*size.x;
	out[tid] = hsv2rgb((float4)(in[tid], 1, 1, 1));
}