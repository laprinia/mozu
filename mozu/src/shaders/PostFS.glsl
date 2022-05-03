#version 460 core
out vec4 fragmentColor;
in vec2 outTextCoord;
uniform sampler2D RTtexture;
uniform float lightExposure;

void main()
{
  vec3 color = texture(RTtexture, outTextCoord).rgb;
  vec3 hdr = vec3(1.0) - exp(-color * lightExposure);
  hdr = pow(hdr, vec3(1.0/2.2));
  fragmentColor = vec4(hdr, 1.0);
} 