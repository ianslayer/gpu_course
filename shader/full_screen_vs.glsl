#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texcoord;

out vec2 vs_fs_texcoord;

void main(void)
{
	gl_Position = vec4(position, 1.0);
	vs_fs_texcoord = texcoord;
}