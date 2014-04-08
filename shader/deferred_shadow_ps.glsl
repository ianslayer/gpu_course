#version 330 core

in vec2 vs_fs_texcoord;

uniform sampler2D gbuffer0;
uniform sampler2D sceneDepthMap;
uniform sampler2D shadowMap;

uniform mat4 shadowMapMatrix;
uniform mat4 invViewProjMatrix;
uniform vec4 lightPosDir;

out vec4 out_color;

vec2 GetShadowOffsets(vec3 N, vec3 L) {
    float cos_alpha = clamp(dot(N, L), 0, 1);
    float offset_scale_N = sqrt(1 - cos_alpha*cos_alpha); // sin(acos(L·N))
    float offset_scale_L = offset_scale_N / cos_alpha;    // tan(acos(L·N))
    return vec2(offset_scale_N, min(2, offset_scale_L));
}


float shadow_offset_lookup(vec4 shadowCoord, vec2 shadowOffset, vec2 offset)
{

	shadowCoord /= shadowCoord.w;
	float shadowBias = shadowOffset.y * 0.0001;
	
	if(shadowCoord.x <= 1.01 && shadowCoord.x >=  -0.01 && shadowCoord.y <= 1.01 && shadowCoord.y >=  -0.01  && shadowCoord.z >=  0 && shadowCoord.z <=  1.0 && texture(shadowMap, shadowCoord.xy + offset ) .r + shadowBias <  min(shadowCoord.z, 1.f)  )
		return 1.0f;
	return 0.0f;
}

void main(void)
{
    vec4 world_normal ;//= texture(gbuffer0, vs_fs_texcoord);
	float depth = texture(sceneDepthMap, vs_fs_texcoord).r;
	
	vec4 world_pos = invViewProjMatrix * vec4(2.0 * vs_fs_texcoord - vec2(1.0),  2.0 * depth -1.0, 1.0);
	world_pos/=world_pos.w;
	vec3 world_pos_dx = normalize(dFdx(world_pos.xyz));
	vec3 world_pos_dy = normalize(dFdy(world_pos.xyz));
	world_normal.xyz = cross(world_pos_dx, world_pos_dy);
	
	vec3 lightDir = normalize(lightPosDir.xyz - world_pos.xyz * lightPosDir.w);
	
	vec2 shadow_offset = GetShadowOffsets(world_normal.xyz, lightDir);
	
	vec3 normalOffset = world_normal.xyz * shadow_offset.x * 0.2;
    vec4 shadow_pos = (shadowMapMatrix * vec4(world_pos.xyz +  normalOffset, 1.0));
	
    float shadow = 0.0;
	
	 ivec2 shadowMapSize = textureSize(shadowMap, 0);
	vec2 pixelOffset = vec2(1.f) / shadowMapSize;
	
	//for(float y = -1.5; y <=1.5; y+=1.0)
		//for(float x = -1.5; x <= 1.5; x+=1.0)
			shadow += shadow_offset_lookup(shadow_pos, shadow_offset, vec2(0, 0) * pixelOffset);
    
	//shadow /=16.0;
	out_color = vec4(shadow);
}