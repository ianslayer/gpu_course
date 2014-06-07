#version 330 core

in vec2 vs_fs_texcoord;

uniform sampler2D gbuffer0;
uniform sampler2D sceneDepthMap;
uniform sampler2D varianceShadowMap;
uniform sampler2DShadow shadowMap;
uniform sampler2DShadow singleSampleShadowMap;
uniform sampler2D noiseMap;

uniform mat4 shadowDepthMatrix;
uniform mat4 shadowMapMatrix;
uniform mat4 invViewProjMatrix;
uniform vec4 lightPosDir;

uniform float minDepth;
uniform float maxDepth;

uniform int shadowTech;
const int SHADOW_MAP_PCF = 0;
const int SHADOW_VARIANCE_SHADOW_MAP = 1;


// For Poisson Disk PCF sampling
const vec2 PoissonSamples[64] = vec2[64]
(
    vec2(0.3928624f, -0.4417621f),
    vec2(-0.1658188f, -0.9695674f),
    vec2(0.3514056f, 0.09865579f),
    vec2(0.9055501f, 0.3758393f),
    vec2(-0.9466816f, -0.2014508f),
    vec2(0.2271994f, -0.6163502f),
    vec2(0.9705266f, -0.1143304f),
    vec2(-0.4241052f, 0.5581087f),
    vec2(-0.7552931f, -0.2426507f),
    vec2(0.6813727f, -0.2424808f),
    vec2(0.7599946f, 0.1809109f),
    vec2(0.1749884f, -0.4202175f),
    vec2(0.2001408f, -0.808381f),
    vec2(-0.1020106f, 0.6724468f),
    vec2(-0.09640376f, 0.9843736f),
    vec2(0.7418533f, -0.6667366f),
    vec2(0.4060591f, -0.7100726f),
    vec2(0.04146584f, 0.8313184f),
    vec2(0.7682328f, -0.07273844f),
    vec2(-0.9879128f, 0.1113683f),
    vec2(0.1507788f, 0.4204168f),
    vec2(0.1268544f, -0.9874692f),
    vec2(0.1975043f, 0.2221317f),
    vec2(-0.2483695f, 0.7942952f),
    vec2(-0.6517572f, -0.07476326f),
    vec2(-0.5938849f, -0.6895654f),
    vec2(0.573212f, -0.51544f),
    vec2(-0.5119625f, -0.4827938f),
    vec2(-0.7136765f, -0.4496614f),
    vec2(0.149394f, 0.6650763f),
    vec2(-0.2171264f, -0.4768726f),
    vec2(-0.8706837f, 0.3010679f),
    vec2(-0.6707711f, 0.4912741f),
    vec2(-0.3448896f, -0.9046497f),
    vec2(0.1558783f, -0.08460935f),
    vec2(-0.5082307f, 0.1079806f),
    vec2(-0.3859636f, 0.3363545f),
    vec2(0.3492522f, 0.5924662f),
    vec2(-0.6982038f, 0.1904326f),
    vec2(0.02703013f, -0.6010728f),
    vec2(0.5838975f, 0.1054943f),
    vec2(0.7713396f, -0.4713659f),
    vec2(-0.0684978f, 0.4461993f),
    vec2(0.6155105f, 0.3245716f),
    vec2(-0.8409063f, -0.03465778f),
    vec2(0.5663417f, 0.7708698f),
    vec2(-0.42215f, -0.2024607f),
    vec2(0.2312844f, 0.8725659f),
    vec2(0.4216993f, 0.9002838f),
    vec2(0.9670017f, 0.1293385f),
    vec2(-0.1041822f, -0.02521214f),
    vec2(0.3780522f, 0.3478679f),
    vec2(0.003256182f, 0.138135f),
    vec2(-0.3148003f, -0.7047654f),
    vec2(0.4262091f, -0.9013284f),
    vec2(0.5679897f, 0.5343465f),
    vec2(0.7375497f, 0.6691415f),
    vec2(0.3956799f, -0.1469177f),
    vec2(-0.08429877f, -0.2316298f),
    vec2(0.9015037f, -0.3306949f),
    vec2(0.0005130528f, -0.8058334f),
    vec2(-0.3042712f, -0.02195431f),
    vec2(-0.5085648f, 0.7534177f),
    vec2(-0.1925334f, 0.1787288f)
);

out float out_color;

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
	//if(shadowPos.x <= 1.001 && shadowPos.x >=  -0.001 && shadowPos.y <= 1.001 && shadowPos.y >=  -0.001  && shadowPos.z >=  0 && shadowPos.z <=  1.0 )
	//{
		vec2 occluder = textureGrad(varianceShadowMap, shadowPos.xy, shadowPosDx.xy, shadowPosDy.xy).xy;
		//vec2 occluder = textureOffset(varianceShadowMap, shadowPos.xy, ivec2(0), 0.5).xy;
		return  ChebyshevUpperBound(occluder, shadowPos.z, 0.0001f, 0.9);
	//}	
	
	//return 1.0;
}

float shadow_offset_lookup(vec3 shadowCoord, vec2 shadowOffset, vec2 offset)
{

	float shadowBias = shadowOffset.y * 0.00001  ;
	
	//if(shadowCoord.x <= 1.001 && shadowCoord.x >=  -0.001 && shadowCoord.y <= 1.001 && shadowCoord.y >=  -0.001  && shadowCoord.z >=  0 && shadowCoord.z <=  1.0 )
	//{
		return texture(shadowMap, vec3(shadowCoord.xy + offset, shadowCoord.z - shadowBias) ) ;
	//}
	//return 1.0f;
}

float RandomDiscPCFShadow(vec3 shadowPos, vec2 shadowOffset)
{
    float shadow = 0.0;
	float numSamples = 8;
	 ivec2 shadowMapSize = textureSize(shadowMap, 0);	
	 vec2 filterSize = vec2(2.f) / vec2(shadowMapSize);
	 
	 ivec2 noiseMapSize = textureSize(noiseMap, 0);
	 ivec2 noiseSamplePos = ivec2(gl_FragCoord.xy) % noiseMapSize;
	 
	 float theta = texelFetch(noiseMap, noiseSamplePos, 0).r * 2.0 * 3.14;
	 mat2 randomRotationMatrix = mat2(cos(theta), -sin(theta), sin(theta), cos(theta));
	 
	 
	 for(int i = 0; i < int(numSamples); ++i)
	 {
		shadow += shadow_offset_lookup(shadowPos, shadowOffset, filterSize * ( randomRotationMatrix * PoissonSamples[i]));
	 }
	 
	 return shadow / numSamples;
	 
}

float PCFShadow(vec3 shadowPos, vec2 shadowOffset)
{
    float shadow = 0.0;
	

	shadow = shadow_offset_lookup(shadowPos, shadowOffset, vec2(0, 0));
    
	
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

	/*
	float uw0 = (4 - 3 * s);
	float uw1 = 7;
	float uw2 = (1 + 3 * s);

	float u0 = (3 - 2 * s) / uw0 - 2;
	float u1 = (3 + s) / uw1;
	float u2 = s / uw2 + 2;

	float vw0 = (4 - 3 * t);
	float vw1 = 7;
	float vw2 = (1 + 3 * t);

	float v0 = (3 - 2 * t) / vw0 - 2;
	float v1 = (3 + t) / vw1;
	float v2 = t / vw2 + 2;

	shadow += uw0 * vw0 * shadow_offset_lookup(vec3(baseUV, shadowPos.z), shadowOffset, vec2(u0, v0) * shadowMapInvSize);
	shadow += uw1 * vw0 * shadow_offset_lookup(vec3(baseUV, shadowPos.z), shadowOffset, vec2(u1, v0) * shadowMapInvSize);
	shadow += uw2 * vw0 * shadow_offset_lookup(vec3(baseUV, shadowPos.z), shadowOffset, vec2(u2, v0) * shadowMapInvSize);

	shadow += uw0 * vw1 * shadow_offset_lookup(vec3(baseUV, shadowPos.z), shadowOffset, vec2(u0, v1) * shadowMapInvSize);
	shadow += uw1 * vw1 * shadow_offset_lookup(vec3(baseUV, shadowPos.z), shadowOffset, vec2(u1, v1) * shadowMapInvSize);
	shadow += uw2 * vw1 * shadow_offset_lookup(vec3(baseUV, shadowPos.z), shadowOffset, vec2(u2, v1) * shadowMapInvSize);

	shadow += uw0 * vw2 * shadow_offset_lookup(vec3(baseUV, shadowPos.z), shadowOffset, vec2(u0, v2) * shadowMapInvSize);
	shadow += uw1 * vw2 * shadow_offset_lookup(vec3(baseUV, shadowPos.z), shadowOffset, vec2(u1, v2) * shadowMapInvSize);
	shadow += uw2 * vw2 * shadow_offset_lookup(vec3(baseUV, shadowPos.z), shadowOffset, vec2(u2, v2) * shadowMapInvSize);

	shadow /= 144;	
*/
	return shadow;
}

void main(void)
{
    vec3 worldNormal = normalize(texture(gbuffer0, vs_fs_texcoord).xyz );
	float depth = texture(sceneDepthMap, vs_fs_texcoord).r;
	
	vec4 worldPos = invViewProjMatrix * vec4(2.0 * vs_fs_texcoord - vec2(1.0),  2.0 * depth -1.0, 1.0);
	
	worldPos/=worldPos.w;
	vec3 worldPosDx = normalize(dFdx(worldPos.xyz));
	vec3 worldPosDy = normalize(dFdy(worldPos.xyz));

	//worldNormal = normalize(cross(worldPosDx, worldPosDy));
	
	vec3 lightDir = normalize(lightPosDir.xyz - worldPos.xyz * lightPosDir.w);
	
	vec2 shadowOffset = GetShadowOffsets(worldNormal.xyz, lightDir);
	
	vec3 normalOffset = vec3(0);
	if(shadowTech == SHADOW_MAP_PCF)
		normalOffset = worldNormal.xyz * shadowOffset.x * 2;
    vec4 shadowPos = (shadowMapMatrix * vec4(worldPos.xyz +  normalOffset, 1.0));

	shadowPos.xyz /= shadowPos.w;
	shadowPos.z = ((shadowDepthMatrix * vec4(worldPos.xyz +  normalOffset, 1.0)).z);
	shadowPos.z = (shadowPos.z - minDepth) / (maxDepth - minDepth);
	
	float shadow = 1;
	if(shadowTech == SHADOW_MAP_PCF)
	{
		if(shadowPos.x <= 1.001 && shadowPos.x >=  -0.001 && shadowPos.y <= 1.001 && shadowPos.y >=  -0.001  && shadowPos.z >=  0 && shadowPos.z <=  1.0 )
			shadow = OptimizedPCFShadow(shadowPos.xyz, shadowOffset);
	}
	else if(shadowTech == SHADOW_VARIANCE_SHADOW_MAP)
	{
		if(shadowPos.x <= 1.001 && shadowPos.x >=  -0.001 && shadowPos.y <= 1.001 && shadowPos.y >=  -0.001  && shadowPos.z >=  0 && shadowPos.z <=  1.0 )
			shadow = SampleVarianceShadowMap(shadowPos.xyz, dFdx(shadowPos.xyz), dFdy(shadowPos.xyz));
	}
	out_color = shadow;
}