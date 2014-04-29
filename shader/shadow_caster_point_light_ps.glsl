#version 330 core

out vec2 out_color;
uniform sampler2D diffuseMap;
uniform float minDepth;
uniform float maxDepth;
in vec2 vs_fs_texcoord;
in vec4 shadowPos;

void main(void)
{
	//out_color = texture(diffuseMap, vs_fs_texcoord);

	float linearZ = (shadowPos.z - minDepth)/ (maxDepth - minDepth);
	out_color = vec2(linearZ, linearZ * linearZ);
	gl_FragDepth = linearZ;
}