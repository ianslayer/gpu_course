#version 330 core

out vec4 out_color;
uniform sampler2D diffuseMap;
in vec2 vs_fs_texcoord;
in vec4 shadowPos;

void main(void)
{
	//out_color = texture(diffuseMap, vs_fs_texcoord);
	
	//float linearZ = 0.5 * (shadowPos.z + 1.0) ;
	
	out_color = vec4(gl_FragCoord.z, gl_FragCoord.z * gl_FragCoord.z, 0, 0);
	//gl_FragDepth = linearZ / 3000;
}