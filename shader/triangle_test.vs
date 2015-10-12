#version 410 core

layout(location = 0) in vec2 vx_pos;
layout(location = 1) in vec4 vx_color;

uniform vec2 scale;

out Vertex
{
  vec4 color;
} vertex;

void main()
{
    gl_Position = vec4(vx_pos.x * scale.x - 1, vx_pos.y * scale.y - 1, 0, 1);
    vertex.color = vx_color;
}  