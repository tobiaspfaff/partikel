#version 410 core

out vec4 color;
//in vec2 vertex_uv;
in vec4 vertex_color;

void main(void)
{
    color = vertex_color;
}
