attribute vec3 color;
attribute vec3 position;

varying vec3 outputColor;

uniform mat4 scale;
uniform mat4 offset;

void main(void)
{
	gl_Position = offset * scale * vec4( position, 1.0);
	outputColor = color;
}