#version 410 core

layout(location = 0) in vec4 vx_data;

uniform vec2 scale;
uniform ivec2 size;

out Vertex
{
  vec4 color;
} vertex;

void main()
{
	int y = gl_VertexID / size.x;
	int x = gl_VertexID - y * size.x;
    gl_Position = vec4((x+0.5) * scale.x - 1, (y+0.5) * scale.y - 1, 0, 1);
    vertex.color = vx_data;
}  