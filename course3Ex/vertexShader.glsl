attribute vec3 color;
attribute vec3 position;

varying vec3 outputColor;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform vec3 cubeColor;

void main(void)
{
	position.z;
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4( position, 1.0);
	outputColor = cubeColor;
}