#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texcoord;

uniform mat4 modelMatrix;
uniform mat4 shadowViewMatrix;
uniform mat4 shadowMapMatrix;
out vec2 vs_fs_texcoord;
out vec4 shadowPos;

void main(void)
{
	vs_fs_texcoord = texcoord;
	vec4 worldPos = modelMatrix * vec4( position, 1.0);
	shadowPos = shadowMapMatrix * worldPos;
	gl_Position = shadowMapMatrix * worldPos;
}