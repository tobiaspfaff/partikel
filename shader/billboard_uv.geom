#version 410

layout (points) in;
layout (triangle_strip) out;
layout (max_vertices = 4) out;

uniform vec2 scale;
uniform float billboard_size;

out vec2 vertex_uv;
out vec4 vertex_color;

in Vertex
{
 	vec4 color;
} vertex[];

void main()
{
	vec2 pos = gl_in[0].gl_Position.xy;
	vec2 sz = vec2(billboard_size*scale.x, billboard_size*scale.y);
	
	// left-bottom
	gl_Position = vec4(pos.x - sz.x, pos.y - sz.y, 0, 1);
	vertex_uv = vec2(0,0);
	vertex_color = vertex[0].color;
	EmitVertex();

	// left-top
	gl_Position = vec4(pos.x - sz.x, pos.y + sz.y, 0, 1);
	vertex_uv = vec2(0,1);
	vertex_color = vertex[0].color;
	EmitVertex();

	// right-bottom
	gl_Position = vec4(pos.x + sz.x, pos.y - sz.y, 0, 1);
	vertex_uv = vec2(1,0);
	vertex_color = vertex[0].color;
	EmitVertex();

	// right-top
	gl_Position = vec4(pos.x + sz.x, pos.y + sz.y, 0, 1);
	vertex_uv = vec2(1,1);
	vertex_color = vertex[0].color;
	EmitVertex();

	EndPrimitive();
}