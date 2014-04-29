#version 330 core

in vec2 vs_fs_texcoord;
in vec3 world_pos;
in vec3 world_normal;
in vec4 vs_fs_tangent;
in vec4 shadow_pos;

in vec3 tangentView;
in vec4 tangentLight;

uniform vec2 window_size;

out vec4 out_color;

uniform bool useTangentLight;
uniform bool useDeferredShadow;
uniform vec4 lightPosDir;
uniform vec3 world_cam_pos;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D specularMap;
uniform sampler2D maskMap;
uniform sampler2D shadowMap;

uniform bool useMask;

uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 specular;
uniform float roughness;

uniform float lightRadius;
uniform vec3 lightIntensity;

uniform int dbgShowMode;
const int DBG_DRAW_NONE = 0;
const int DBG_DRAW_UV_TILING = 1;
const int DBG_DRAW_TANGENT_SPACE = 2;
const int DBG_DRAW_DIFFUSE = 3;
const int DBG_DRAW_NORMAL = 4;
const int DBG_DRAW_SPECULAR = 5;
const int DBG_DRAW_DIFFUSE_LIGHTING = 6;
const int DBG_DRAW_SPECULAR_LIGHTING = 7;
const int DBG_DRAW_FRESNEL_SPECULAR_LIGHTING = 8;

void main(void)
{
	vec3 normal = vec3(0.5);
	vec3 halfVector = vec3(0);
	vec3 lightDir = vec3(0);
	float distToLight = distance(lightPosDir.xyz, world_pos);
	
	if(useTangentLight)
	{
        lightDir = tangentLight.xyz;
		 normal = normalize( texture(normalMap, vs_fs_texcoord).xyz * 2.0 - vec3(1.0));
		 halfVector = normalize(normalize(tangentLight.xyz) + normalize(tangentView));
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
	float lDotH = max(dot(normalize(lightDir), halfVector), 0);

	//Blinn Phong NDF
    float alpha = pow(roughness, texture(specularMap, vs_fs_texcoord).r );
    alpha *= alpha; // this is only to exaggerate specular light, it has no physical meaning
    float specular = ((alpha + 2.0) / (2 * 3.14) ) * pow(nDotH, alpha);

    //GGX NDF
    alpha = pow(roughness, 1.0 - texture(specularMap, vs_fs_texcoord).r );
    alpha /= roughness;
    float beta = ((nDotH * nDotH) * (alpha * alpha - 1) + 1);
    beta *= beta;
    specular = alpha * alpha / (3.14 * beta);
    
    //Fresnel
    vec4 F0 = vec4(0.04);
    vec4 FSchlick = F0 + (vec4(1.0) - F0) * pow((1.0 - lDotH), 5 );
    
	float distanceAtt = 1.0;
	if(lightPosDir.w == 1.0) //this is point light
		distanceAtt = 1.0/ ( (1.0 + distToLight / lightRadius ) * (1.0 + distToLight / lightRadius ) ); //see http://imdoingitwrong.wordpress.com/2011/01/31/light-attenuation/ for details
    
	float shadow = 1.f;
	
	if(useDeferredShadow)
	{
		vec2 vpTexcoord = gl_FragCoord.xy / window_size;
		shadow = texture(shadowMap, vpTexcoord).r;
	}
	else
	{
		vec4 shadow_coord = shadow_pos ;/// shadow_pos.w;
		if(textureProj(shadowMap, shadow_coord).r <  min(shadow_coord.z, 1.f))
			shadow = 0.3f;
	}
	
	out_color =shadow* distanceAtt * vec4(lightIntensity, 1.0) * nDotL * ((vec4(1.0) - FSchlick) *texture(diffuseMap, vs_fs_texcoord) / 3.14 +  FSchlick * vec4(specular) ) ;
    
	if(useMask)
	{
		 if(texture(maskMap, vs_fs_texcoord).r == 0 )
			discard;
	}
	
    //debug output
    if(dbgShowMode == DBG_DRAW_UV_TILING)
    {
        out_color = 0.7 * vec4( fract(vs_fs_texcoord), 0, 0 ) + 0.3 * texture(diffuseMap, vs_fs_texcoord);
    }
    else if(dbgShowMode == DBG_DRAW_TANGENT_SPACE)
    {
        out_color = vec4( (normalize(vs_fs_tangent) + vec4(1.0)) *0.5);
    }
    else if(dbgShowMode == DBG_DRAW_DIFFUSE)
    {
        out_color = texture(diffuseMap, vs_fs_texcoord);
    }
    else if(dbgShowMode == DBG_DRAW_NORMAL)
    {
        out_color = texture(normalMap, vs_fs_texcoord);
    }
    else if(dbgShowMode == DBG_DRAW_SPECULAR)
    {
        out_color = texture(specularMap, vs_fs_texcoord);
    }
    else if(dbgShowMode == DBG_DRAW_DIFFUSE_LIGHTING)
    {
        out_color =  distanceAtt * vec4(lightIntensity, 1.0) * nDotL * ((vec4(1.0) - FSchlick) *texture(diffuseMap, vs_fs_texcoord) ) / 3.14;
    }
    else if(dbgShowMode == DBG_DRAW_SPECULAR_LIGHTING)
    {
          out_color =  distanceAtt * vec4(lightIntensity, 1.0) * nDotL * vec4(specular);
    }
    else if(dbgShowMode == DBG_DRAW_FRESNEL_SPECULAR_LIGHTING)
    {
          out_color =  distanceAtt * vec4(lightIntensity, 1.0) * nDotL * FSchlick * vec4(specular);
    }

}