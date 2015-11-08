#version 410 core

layout(location = 0) in float vx_px;
layout(location = 1) in float vx_py;

uniform vec2 scale;

out Vertex
{
  vec4 color;
} vertex;

void main()
{
    gl_Position = vec4(vx_px * scale.x - 1, vx_py * scale.y - 1, 0, 1);
	vertex.color = vec4(0,0,1,1); //vx_color;
}  