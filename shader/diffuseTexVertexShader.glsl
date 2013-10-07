#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec2 vs_fs_texcoord;

void main(void)
{
	position.z;
	vs_fs_texcoord = texcoord;
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4( position, 1.0);
}