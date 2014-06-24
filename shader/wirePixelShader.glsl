#version 330 core

out vec4 out_color;
in vec3 vs_fs_color;

void main(void)
{
	out_color = vec4(vs_fs_color, 1.0);
}