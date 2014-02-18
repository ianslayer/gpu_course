#version 330 core


out vec4 out_color;
uniform sampler2D diffuseMap;
in vec2 vs_fs_texcoord;

void main(void)
{
	out_color = texture(diffuseMap, vs_fs_texcoord);
}