#version 460 core
out vec4 fragmentColor;
in vec2 textureCoord;
uniform sampler2D RTtexture;

void main()
{
vec3 color = texture(RTtexture, textureCoord).rgb;
    color = pow(color, vec3(1.0/2.2));
    fragmentColor = vec4(color, 1.0);
}