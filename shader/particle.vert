#version 410 core

layout(location = 0) in vec2 vx_p;

uniform vec2 scale;

out Vertex
{
  vec4 color;
} vertex;

void main()
{
    gl_Position = vec4(vx_p.x * scale.x - 1, vx_p.y * scale.y - 1, 0, 1);
	vertex.color = vec4(0,0,1,1); //vx_color;
}  