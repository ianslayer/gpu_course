#version 330 core

in vec2 vs_fs_texcoord;
in vec3 world_pos;
in vec3 world_normal;

uniform sampler2D diffuseTex;


void main(void)
{
	gl_FragColor = texture(diffuseTex, vs_fs_texcoord);
}