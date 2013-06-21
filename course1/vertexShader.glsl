attribute vec3 color;
attribute vec3 position;
varying vec3 outputColor;

void main(void)
{
	gl_Position = vec4(position, 1.0);
	outputColor = color;
}