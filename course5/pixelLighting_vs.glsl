attribute vec3 normal;
attribute vec3 position;
attribute vec3 color;
attribute vec2 texcoord;

varying vec2 vTexcoord;

varying vec3 vViewDir;
varying vec3 vViewLightDir;
varying vec3 vViewNormal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

uniform vec4 lightPosDir;


void main(void)
{
	position.z;
	
	vec4 worldPos = modelMatrix * vec4( position, 1.0);
    vec4 viewPos =  viewMatrix * worldPos;

    vec3 viewDir = normalize(vec3(0) - viewPos.xyz);
	vec3 viewLightDir = normalize(viewMatrix * (vec4(lightPosDir.xyz - lightPosDir.w * worldPos.xyz, 0 ) ) );
	vec4 viewNormal = normalize(viewMatrix * modelMatrix * vec4(normal, 0.0) );	

    vViewDir = viewDir;
    vViewLightDir = viewLightDir;
    vViewNormal = viewNormal;
		
    vTexcoord = texcoord;

	gl_Position = projectionMatrix * viewPos;
	
}