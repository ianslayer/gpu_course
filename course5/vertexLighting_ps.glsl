varying vec3 vColor;
varying vec2 vTexcoord;

uniform sampler2D diffuseTexture;

void main(void)
{
    vTexcoord.y *= -1.0;
    float4 diffuseColor = texture2D(diffuseTexture, vTexcoord);

	gl_FragColor = vec4(diffuseColor.rgb * vColor, 1.0);
}