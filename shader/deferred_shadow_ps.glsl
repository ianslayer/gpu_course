#version 330 core

in vec2 vs_fs_texcoord;

uniform sampler2D gbuffer0;
uniform sampler2D sceneDepthMap;
uniform sampler2D shadowMap;

uniform mat4 shadowMapMatrix;
uniform mat4 invViewProjMatrix;
uniform vec4 lightPosDir;

out vec4 out_color;

vec2 get_shadow_offsets(vec3 N, vec3 L) {
    float cos_alpha = clamp(dot(N, L), 0, 1);
    float offset_scale_N = sqrt(1 - cos_alpha*cos_alpha); // sin(acos(L·N))
    float offset_scale_L = offset_scale_N / cos_alpha;    // tan(acos(L·N))
    return vec2(offset_scale_N, min(2, offset_scale_L));
}

void main(void)
{
    vec4 world_normal = texture(gbuffer0, vs_fs_texcoord);
	float depth = texture(sceneDepthMap, vs_fs_texcoord).r;
	
	vec4 world_pos = invViewProjMatrix * vec4(2.0 * vs_fs_texcoord - vec2(1.0),  2.0 * depth -1.0, 1.0);
	world_pos/=world_pos.w;
	vec3 world_pos_dx = normalize(dFdx(world_pos.xyz));
	vec3 world_pos_dy = normalize(dFdy(world_pos.xyz));
	world_normal.xyz = cross(world_pos_dx, world_pos_dy);
	
	vec3 lightDir = normalize(lightPosDir.xyz - world_pos.xyz * lightPosDir.w);
	
	vec2 shadow_offset = get_shadow_offsets(world_normal.xyz, lightDir);
	
    vec4 shadow_pos = (shadowMapMatrix * vec4(world_pos.xyz, 1.0));
    
    float shadow = 0.0;
	vec4 shadow_coord = shadow_pos / shadow_pos.w;
	float offset = (shadow_offset.x * 0.000005 + shadow_offset.y * 0.000005 + 0.00000);
	
    if(texture(shadowMap, shadow_coord.xy).r + offset <  min(shadow_coord.z, 1.f) && shadow_coord.x <= 1.01 && shadow_coord.x >=  -0.01 && shadow_coord.y <= 1.01 && shadow_coord.y >=  -0.01  && shadow_coord.z >=  0 && shadow_coord.z <=  1.0)
		shadow = 1.0f;
    
	out_color = vec4(shadow);
}