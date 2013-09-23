attribute vec3 normal;
attribute vec3 position;
attribute vec3 color;
attribute vec2 texcoord;

varying vec3 vColor;
varying vec2 vTexcoord;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

uniform vec4 lightPosDir;
uniform vec3 lightColor;

uniform vec3 diffuseAlbedo;
uniform vec3 specularAlbedo;
uniform float specularExponent;

vec3 Shade(vec3 normal, vec3 viewDirection, vec3 lightDirection)
{
    float cosTerm = max( dot(normal, lightDirection), 0);
    vec3 halfAngle = normalize(viewDirection + lightDirection);
    float specular = pow( max(dot(normal, halfAngle), 0), specularExponent);

    return lightColor * (diffuseAlbedo + specularAlbedo * specular) * cosTerm;
}

void main(void)
{
	vec4 worldPos = modelMatrix * vec4( position, 1.0);
    vec4 viewPos =  viewMatrix * worldPos;
    vec3 viewDir = vec3(0) - viewPos.xyz;

	vec3 viewLightDir = viewMatrix * (vec4(lightPosDir.xyz - lightPosDir.w * worldPos.xyz, 0 ) );

	vec4 viewNormal = normalize(viewMatrix * modelMatrix * vec4(normal, 0.0) );	

	vColor = Shade(normalize(viewNormal.xyz), normalize(viewDir), normalize(viewLightDir) );
	
    vTexcoord = texcoord;

	gl_Position = projectionMatrix * viewPos;
	
}