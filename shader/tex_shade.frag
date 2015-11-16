#version 410 core

out vec4 color;
in vec4 vertex_color;
in vec2 vertex_uv;
uniform sampler2D tex0;

void main(void)
{
    color = texture(tex0, vertex_uv) * vertex_color;
}
