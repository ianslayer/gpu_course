varying vec2 vTexcoord;

varying vec3 vViewDir;
varying vec3 vViewLightDir;
varying vec3 vViewNormal;

uniform vec3 diffuseAlbedo;
uniform vec3 specularAlbedo;
uniform float specularExponent;

uniform vec3 lightColor;

vec3 Shade(vec3 normal, vec3 viewDirection, vec3 lightDirection)
{
    float cosTerm = max( dot(normal, lightDirection), 0);
    vec3 halfAngle = normalize(viewDirection + lightDirection);
    float specular =  pow( max(dot(normal, halfAngle), 0), specularExponent);

    return lightColor * (diffuseAlbedo + specularAlbedo * specular) * cosTerm;
}

uniform sampler2D diffuseTexture;

void main(void)
{
    vTexcoord.y *= -1.0;
    float4 diffuseColor = texture2D(diffuseTexture, vTexcoord);

    vec3 lighting = Shade(normalize(vViewNormal), normalize(vViewDir), normalize(vViewLightDir) );

	gl_FragColor = vec4(diffuseColor.rgb * lighting, 1.0);
}