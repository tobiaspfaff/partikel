#version 410 core

layout(location = 0) in vec4 vx_pos;

void main()
{
    gl_Position = vx_pos;
}  