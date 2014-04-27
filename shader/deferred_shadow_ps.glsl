#version 330 core

in vec2 vs_fs_texcoord;

uniform sampler2D gbuffer0;
uniform sampler2D sceneDepthMap;
uniform sampler2D varianceShadowMap;
uniform sampler2DShadow shadowMap;

uniform mat4 shadowViewMatrix;
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

float Linstep(float a, float b, float v)
{
    return clamp((v - a) / (b - a), 0, 1);
}

// Reduces VSM light bleedning
float ReduceLightBleeding(float pMax, float amount)
{
  // Remove the [0, amount] tail and linearly rescale (amount, 1].
   return Linstep(amount, 1.0f, pMax);
}

float ChebyshevUpperBound(vec2 moments, float mean, float minVariance,
                          float lightBleedingReduction)
{
    // Compute variance
    float variance = moments.y - (moments.x * moments.x);
    variance = max(variance, minVariance);

    // Compute probabilistic upper bound
    float d = mean - moments.x;
    float pMax = variance / (variance + (d * d));

    pMax = ReduceLightBleeding(pMax, lightBleedingReduction);

    // One-tailed Chebyshev
    return (mean <= moments.x ? 1.0f : pMax);
}

float SampleVarianceShadowMap(vec3 shadowPos, vec3 shadowPosDx, vec3 shadowPosDy)
{
	if(shadowPos.x <= 1.01 && shadowPos.x >=  -0.01 && shadowPos.y <= 1.01 && shadowPos.y >=  -0.01  && shadowPos.z >=  0 && shadowPos.z <=  1.0 )
	{
		vec2 occluder = textureGrad(varianceShadowMap, shadowPos.xy, shadowPosDx.xy, shadowPosDy.xy).xy;
			return  1 - ChebyshevUpperBound(occluder, shadowPos.z, 0.0001f, 0);
	}	
	
	return 0;
}

float shadow_offset_lookup(vec3 shadowCoord, vec2 shadowOffset, vec2 offset)
{

	float shadowBias = shadowOffset.y * 0.00001  ;
	
	if(shadowCoord.x <= 1.01 && shadowCoord.x >=  -0.01 && shadowCoord.y <= 1.01 && shadowCoord.y >=  -0.01  && shadowCoord.z >=  0 && shadowCoord.z <=  1.0 )
	{
		return texture(shadowMap, vec3(shadowCoord.xy + offset, shadowCoord.z - shadowBias) ) ;
	}
	return 0.0f;
}

float PCFShadow(vec3 shadowPos, vec2 shadowOffset)
{
    float shadow = 0.0;
	
	 ivec2 shadowMapSize = textureSize(shadowMap, 0);

	 vec2 pixelOffset = vec2(1.f) / vec2(shadowMapSize);
	

	shadow = shadow_offset_lookup(shadowPos, shadowOffset, vec2(0, 0) * pixelOffset);
    
	
	return shadow;
}

float NaivePCFShadow(vec3 shadowPos, vec2 shadowOffset)
{
    float shadow = 0.0;
	
	 ivec2 shadowMapSize = textureSize(shadowMap, 0);

	 vec2 pixelOffset = vec2(1.f) / vec2(shadowMapSize);
	
	//naive implementation of pcf filter
	for(float y = -1.0; y <=1.0; y+=1.0)
		for(float x = -1.0; x <= 1.0; x+=1.0)
			shadow += shadow_offset_lookup(shadowPos, shadowOffset, vec2(x, y) * pixelOffset);
    
	shadow /=9.0;
	
	return shadow;
}

float OptimizedPCFShadow(vec3 shadowPos, vec2 shadowOffset)
{
	float shadow = 0.0;
	ivec2 shadowMapSize = textureSize(shadowMap, 0);
	vec2 uv = shadowPos.xy * vec2(shadowMapSize);
	vec2 shadowMapInvSize = 1.0 / vec2(shadowMapSize);
	vec2 baseUV;
	baseUV.x = floor(uv.x + 0.5);
	baseUV.y = floor(uv.y + 0.5);
	
	float s = uv.x + 0.5 - baseUV.x;
	float t = uv.y + 0.5 - baseUV.y;
	
	baseUV -= vec2(0.5, 0.5);
	baseUV *= shadowMapInvSize;
	
	float uw0 = (3 - 2 * s);
	float uw1 = (1 + 2 * s);

	float u0 = (2 - s) / uw0 - 1;
	float u1 = s / uw1 + 1;

	float vw0 = (3 - 2 * t);
	float vw1 = (1 + 2 * t);

	float v0 = (2 - t) / vw0 - 1;
	float v1 = t / vw1 + 1;
	
	shadow += uw0 * vw0 * shadow_offset_lookup(vec3(baseUV, shadowPos.z), shadowOffset, vec2(u0, v0) * shadowMapInvSize );
	shadow += uw1 * vw0 * shadow_offset_lookup(vec3(baseUV, shadowPos.z), shadowOffset, vec2(u1, v0) * shadowMapInvSize );
	shadow += uw0 * vw1 * shadow_offset_lookup(vec3(baseUV, shadowPos.z), shadowOffset, vec2(u0, v1) * shadowMapInvSize );
	shadow += uw1 * vw1 * shadow_offset_lookup(vec3(baseUV, shadowPos.z), shadowOffset, vec2(u1, v1) * shadowMapInvSize );
	shadow/=16;
	return shadow;
}

void main(void)
{
    vec3 worldNormal;// = normalize(texture(gbuffer0, vs_fs_texcoord).xyz );
	float depth = texture(sceneDepthMap, vs_fs_texcoord).r;
	
	vec4 worldPos = invViewProjMatrix * vec4(2.0 * vs_fs_texcoord - vec2(1.0),  2.0 * depth -1.0, 1.0);
	
	worldPos/=worldPos.w;
	vec3 worldPosDx = normalize(dFdx(worldPos.xyz));
	vec3 worldPosDy = normalize(dFdy(worldPos.xyz));
	worldNormal = normalize(cross(worldPosDx, worldPosDy));
	
	vec3 lightDir = normalize(lightPosDir.xyz - worldPos.xyz * lightPosDir.w);
	
	vec2 shadowOffset = GetShadowOffsets(worldNormal.xyz, lightDir);
	
	vec3 normalOffset = worldNormal.xyz * shadowOffset.x * 0.0;
    vec4 shadowPos = (shadowMapMatrix * vec4(worldPos.xyz +  normalOffset, 1.0));
	//float linearZ = shadowPos.w;
	shadowPos.xyz /= shadowPos.w;
	//shadowPos.w /= 3000;
    float shadow = OptimizedPCFShadow(shadowPos.xyz, shadowOffset);
	
	
	//float shadow = SampleVarianceShadowMap(shadowPos.xyz, dFdx(shadowPos.xyz), dFdy(shadowPos.xyz));
	
	out_color = vec4(shadow);
}