#version 330 core

in vec2 vs_fs_texcoord;

uniform sampler2D source;
uniform int direction; //0 == horizontal blur, 1 == vertical blur
out vec3 out_color;

vec3 FastGaussianBlurHori()
{
	const float offset[3] = float[3](0.638336, 2.348645, 4.245085);
	const float weight[3] = float[3](0.277281, 0.186763, 0.035956);
	vec2 sourceSize = vec2(textureSize(source,0));
	vec3 color = vec3(0);
	for(int i = 0; i < 3; i++)
	{
		color += texture(source, vs_fs_texcoord + vec2(offset[i]/sourceSize.x, 0) ).xyz * weight[i];
		color += texture(source, vs_fs_texcoord - vec2(offset[i]/sourceSize.x, 0) ).xyz * weight[i];
	}
	
	return color;
}

vec3 FastGaussianBlurVert()
{
	const float offset[3] = float[3](0.638336, 2.348645, 4.245085);
	const float weight[3] = float[3](0.277281, 0.186763, 0.035956);
	vec2 sourceSize = vec2(textureSize(source,0));
	vec3 color = vec3(0);
	for(int i = 0; i < 3; i++)
	{
		color += texture(source, vs_fs_texcoord + vec2(0, offset[i]/sourceSize.y) ).xyz * weight[i];
		color += texture(source, vs_fs_texcoord - vec2(0, offset[i]/sourceSize.y) ).xyz * weight[i];
	}
	
	return color;
}

vec3 GaussianBlurHori()
{
	const float offset[5] = float[5](1, 2, 3, 4, 5);
	const float weight[5] = float[5](0.176998, 0.121649, 0.065114, 0.027144, 0.008812);
	
	vec3 color = texture(source, vs_fs_texcoord).xyz * 0.200565;

	vec2 sourceSize = vec2(textureSize(source,0));
	for(int i = 0; i < 5; i++)
	{
		color += texture(source, vs_fs_texcoord + vec2(offset[i]/sourceSize.x, 0) ).xyz * weight[i];
		color += texture(source, vs_fs_texcoord - vec2(offset[i]/sourceSize.x, 0) ).xyz * weight[i];
	}
	
	return color;
}

vec3 GaussianBlurVert()
{
	const float offset[5] = float[5](1, 2, 3, 4, 5);
	const float weight[5] = float[5](0.176998, 0.121649, 0.065114, 0.027144, 0.008812);
	
	vec3 color = texture(source, vs_fs_texcoord).xyz * 0.200565;
	
	vec2 sourceSize = vec2(textureSize(source,0));
	for(int i = 0; i < 5; i++)
	{
		color += texture(source, vs_fs_texcoord + vec2(0, offset[i]/sourceSize.y) ).xyz * weight[i];
		color += texture(source, vs_fs_texcoord - vec2(0, offset[i]/sourceSize.y) ).xyz * weight[i];
	}
	
	return color;

}

void main(void)
{
	if(direction == 0)
		out_color = FastGaussianBlurHori();
	else if(direction == 1)
		out_color = FastGaussianBlurVert();
	else
		out_color = texture(source, vs_fs_texcoord).xyz;
}