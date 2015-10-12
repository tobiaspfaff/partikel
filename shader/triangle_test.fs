#version 410 core

out vec4 color;
in vec2 vertex_uv;
in vec4 vertex_color;
uniform sampler2D tex0;

void main(void)
{
    color = texture(tex0, vertex_uv).rgba * vertex_color;
}
