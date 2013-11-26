#version 330 core

in vec2 vs_fs_texcoord;
in vec3 world_pos;
in vec3 world_normal;

uniform sampler2D diffuseTex;
uniform sampler2D specularTex;

uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 specular;
uniform float roughness;

uniform vec3 cam_pos;

uniform vec4 lightPosDir;
uniform vec3 lightIntensity;

void main(void)
{
	vec3 lightDir = lightPosDir.xyz - world_pos * lightPosDir.w;
	float distToLight = length(lightDir);
	lightDir /= distToLight;
	
	vec3 viewDir = normalize(cam_pos - world_pos);
	
	float ndotL = max(dot(world_normal, lightDir), 0.f);
	
//	vec h = lightDir + 
	
	gl_FragColor = vec4(lightIntensity, 1.f) * vec4(vec3(ndotL), 1.0) *  vec4(diffuse, 1.f) * texture(diffuseTex, vs_fs_texcoord);//vec4(vs_fs_texcoord, 0.0, 1.0) ;
}