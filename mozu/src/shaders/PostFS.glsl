#version 460 core
out vec4 fragmentColor;
in vec2 outTextureCoord;
uniform sampler2D texture1;

void main() {
    vec3 base = texture(texture1, outTextureCoord).rgb;
    fragmentColor = vec4(base, 1.0);
}

