#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec2 vs_fs_texcoord;

out vec3 world_pos;
out vec3 world_normal;

void main(void)
{
	position.z;
	vs_fs_texcoord = texcoord;
	world_pos = (modelMatrix * vec4(position, 1.0)).xyz;
	world_normal = (modelMatrix * vec4(normal, 0.0)).xyz;
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4( position, 1.0);
}