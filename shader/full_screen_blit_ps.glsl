#version 330 core

in vec2 vs_fs_texcoord;

uniform sampler2D source;

out vec4 out_color;

void main(void)
{
	out_color = texture(source, vs_fs_texcoord);
}