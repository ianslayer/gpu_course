#version 330 core


out vec2 out_color;
uniform sampler2D diffuseMap;
in vec2 vs_fs_texcoord;
in vec4 shadowPos;

void main(void)
{
	//out_color = texture(diffuseMap, vs_fs_texcoord);
	
	out_color = vec2(gl_FragCoord.z, gl_FragCoord.z);
}