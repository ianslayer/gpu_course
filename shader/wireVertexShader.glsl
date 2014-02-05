#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

out vec3 vs_fs_color;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main(void)
{
    vs_fs_color = color;
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4( position, 1.0);
}