#version 330 core

in vec2 vs_fs_texcoord;
uniform sampler2D diffuseTex;

void main(void)
{
	gl_FragColor = texture(diffuseTex, vs_fs_texcoord);//vec4(vs_fs_texcoord, 0.0, 1.0);
}