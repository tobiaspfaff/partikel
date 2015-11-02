#version 410 core

layout(location = 0) in vec2 vx_pos;
layout(location = 1) in vec4 vx_color;

uniform vec2 scale;

out vec4 vertex_color;

void main()
{
	gl_Position = vec4((vx_pos.x+0.5) * scale.x - 1, (vx_pos.y+0.5) * scale.y - 1, 0, 1);
	vertex_color = vx_color;
}  