#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texcoord;

uniform mat4 modelMatrix;
uniform mat4 shadowMapMatrix;
out vec2 vs_fs_texcoord;

void main(void)
{
	vs_fs_texcoord = texcoord;
	gl_Position = shadowMapMatrix * modelMatrix * vec4( position, 1.0);
}