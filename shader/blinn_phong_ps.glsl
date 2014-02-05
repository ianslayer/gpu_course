#version 330 core

in vec2 vs_fs_texcoord;
in vec3 world_pos;
in vec3 world_normal;
in vec4 vs_fs_tangent;

in vec3 tangentView;
in vec3 tangentLight;

out vec4 out_color;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
//uniform sampler2D specularTex;

uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 specular;
uniform float roughness;


uniform vec3 lightIntensity;

void main(void)
{
    vec3 normal = vec3(0, 0, 1.0);
//    vec4(normalize(tangentLight), 1.0);
    vec3 halfVector = normalize(tangentLight + tangentView);
    float nDotH = dot(normal, halfVector);
	out_color = vec4(nDotH * nDotH * nDotH * nDotH);//vec4(dot(normalize(tangentLight), normal ))* texture(diffuseMap, vs_fs_texcoord);
}