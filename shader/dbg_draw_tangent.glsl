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
    out_color = vec4( (normalize(vs_fs_tangent) + vec4(1.0)) *0.5);

}