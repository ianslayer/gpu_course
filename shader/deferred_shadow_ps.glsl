#version 330 core

in vec2 vs_fs_texcoord;

uniform sampler2D scenePos;
uniform sampler2D shadowMap;

uniform mat4 shadowMapMatrix;

out vec4 out_color;

void main(void)
{
    vec3 world_pos = texture(scenePos, vs_fs_texcoord).xyz;
    vec4 shadow_pos = (shadowMapMatrix * vec4(world_pos, 1.0));
    
    float shadow = 0.0;
    if(textureProj(shadowMap, shadow_pos).r <  min(shadow_pos.z, 1.f))
		shadow = 0.7f;
    
	out_color = vec4(shadow);
}