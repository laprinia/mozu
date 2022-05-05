#version 460 core
out vec4 fragmentColor;
in vec2 outTextCoord;
layout(binding=0) uniform sampler2D RTtexture;
layout(binding=1) uniform sampler2D BMtexture;
uniform float lightExposure;
uniform bool bloom;
void main()
{
  vec3 color = texture(RTtexture, outTextCoord).rgb;
  vec3 bloomColor = texture(BMtexture,outTextCoord).rgb;
  if (bloom) {
	color+=bloomColor;
  }
  vec3 hdr = vec3(1.0) - exp(-color * lightExposure);
  hdr = pow(hdr, vec3(1.0/2.0));
  fragmentColor = vec4(hdr,1.0);
} 