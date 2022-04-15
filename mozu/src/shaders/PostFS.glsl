#version 460 core
out vec4 FragColor;

in vec2 TexCoord;
uniform sampler2D ourTexture;

void main()
{
vec3 color = texture(ourTexture, TexCoord).rgb;
    color = pow(color, vec3(1.0/2.2));
    FragColor = vec4(color, 1.0);
}