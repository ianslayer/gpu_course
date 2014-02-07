#version 330 core

in vec2 vs_fs_texcoord;
in vec3 world_pos;
in vec3 world_normal;
in vec4 vs_fs_tangent;

in vec3 tangentView;
in vec3 tangentLight;

out vec4 out_color;

uniform bool useTangentLight;
uniform vec4 lightPosDir;
uniform vec3 world_cam_pos;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D specularMap;
uniform sampler2D mask;

uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 specular;
uniform float roughness;

uniform vec3 lightIntensity;

void main(void)
{
	vec3 normal = vec3(0.5);
	vec3 halfVector = vec3(0.f);
	vec3 lightDir = vec3(0);
	if(useTangentLight)
	{
        lightDir = tangentLight;
		 normal = normalize( texture(normalMap, vs_fs_texcoord).xyz * 2.f - vec3(1.f));
		 halfVector = normalize(normalize(tangentLight) + normalize(tangentView));
	}
	else
	{
		normal = normalize(world_normal);
		lightDir = normalize(lightPosDir.xyz - world_pos * lightPosDir.w);
		vec3 viewDir = normalize(world_cam_pos - world_pos);
		halfVector = normalize(lightDir + viewDir);

	}
			 
    float nDotL = max(dot(normal, normalize(lightDir) ), 0);
    float nDotH = max(dot(normal, halfVector), 0.0001);
    float maxPower = 32;
	
    float roughness = pow(maxPower, texture(specularMap, vs_fs_texcoord).r );
    float specular = ((roughness + 2.0) / (2 * 3.14) ) * pow(nDotH, roughness);

    out_color = nDotL * (texture(diffuseMap, vs_fs_texcoord) +  vec4(specular) );

}