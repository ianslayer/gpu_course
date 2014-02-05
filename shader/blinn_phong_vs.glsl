#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 tangent;
layout (location = 3) in vec2 texcoord;

uniform mat4 modelMatrix;
uniform mat4 invModelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

uniform vec3 world_cam_pos;
uniform vec4 lightPosDir;

out vec2 vs_fs_texcoord;

out vec3 world_pos;
out vec3 world_normal;
out vec4 vs_fs_tangent;

out vec3 tangentView;
out vec3 tangentLight;

void main(void)
{
	vs_fs_texcoord = texcoord;
    vs_fs_tangent = tangent;
	world_pos = (modelMatrix * vec4(position, 1.0)).xyz;
	world_normal = (modelMatrix * vec4(normal, 0.0)).xyz;
    
    vec3 bitangent = cross(normal, tangent.xyz) * tangent.w;
    vec3 modelCamPos = (invModelMatrix * vec4(world_cam_pos, 1.0) ).xyz;
    
    tangentView = modelCamPos - position;
    tangentView = vec3(dot(tangent.xyz, tangentView), dot(bitangent, tangentView), dot(normal, tangentView));
    
    vec4 modelLightDir = invModelMatrix * lightPosDir;
    tangentLight = modelLightDir.xyz - position * lightPosDir.w;
    tangentLight = vec3(dot(tangent.xyz, tangentLight), dot(bitangent, tangentLight), dot(normal, tangentLight) );
    /*
	vec3 lightDir = lightPosDir.xyz - world_pos * lightPosDir.w;
	float distToLight = length(lightDir);
	lightDir /= distToLight;
    */
    
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4( position, 1.0);
}