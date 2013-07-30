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

void main(void)
{
	position.z;
	
	vec4 worldPos = modelMatrix * vec4( position, 1.0);
	vec3 lightDir = lightPosDir.xyz - lightPosDir.w * worldPos.xyz;
	vec4 worldNormal = modelMatrix * vec4(normal, 0.0);	
	
	vColor = dot(normalize(worldNormal.xyz), lightDir); 
	vTexcoord = texcoord;

	gl_Position = projectionMatrix * viewMatrix * worldPos;
	
}