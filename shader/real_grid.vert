#version 410 core

layout(location = 0) in float vx_data;

uniform vec2 scale;
uniform ivec2 sizeOuter;
uniform ivec2 sizeInner;
uniform float mult;
uniform int method;

out Vertex
{
  vec4 color;
} vertex;

float sq(float v) 
{
	return v*v;
}

// http://www.sron.nl/~pault/colourschemes.pdf
// in: [-1,1] out: color4
vec4 red_blue_colorscheme(float v, float alpha) 
{
    v = clamp(0.5 * (v + 1.0), 0.0, 1.0);
    float v2 = sq(v), v3 = v2*v, v4 = v3*v, v5 = v4*v;

    return vec4 (0.237-2.13*v + 26.92*v2-65.5*v3+63.5*v4-22.36*v5,
                 sq((0.572+1.524*v-1.811*v2)/(1-0.291*v+0.1574*v2)),
                 1.0/(1.579-4.03*v+12.92*v2-31.4*v3+48.6*v4-23.36*v5),
				 alpha);
}

vec4 hsv2rgb(vec4 hsv)
{
    vec4 rgb = vec4(0,0,0,0);
    if (hsv.z != 0) 
    {
		while (hsv.x < 0)
			hsv.x++;
    	float hh = (hsv.x - floor(hsv.x)) * 6;    	
    	int i = int(hh);
    	float ff = hh - i;
    	float p = hsv.z * (1.0 - hsv.y);
		float q = hsv.z * (1.0 - hsv.y * ff);
		float t = hsv.z * (1.0 - hsv.y * (1-ff));
		switch(i)
		{
			case 0: rgb = vec4(hsv.z, t, p, hsv.w); break;
			case 1: rgb = vec4(q, hsv.z, p, hsv.w); break;
			case 2: rgb = vec4(p, hsv.z, t, hsv.w); break;
			case 3: rgb = vec4(p, q, hsv.z, hsv.w); break;
			case 4: rgb = vec4(t, p, hsv.z, hsv.w); break;
			default: rgb = vec4(hsv.z, p, q, hsv.w); break;
		}
	}
    return rgb;
}

void main()
{
	int y = gl_VertexID / sizeOuter.x;
	int x = gl_VertexID - y * sizeOuter.x;
    gl_Position = vec4((x+0.5) * scale.x - 1, (y+0.5) * scale.y - 1, 0, 1);
	if (method == 0)
		vertex.color = red_blue_colorscheme(vx_data * mult, 1.0f);
	else
		vertex.color = hsv2rgb(vec4(vx_data * mult, 1, 1, 1.0f));
}  