#version 460 core
uniform sampler2D THtexture;
uniform float resolution;
uniform float radius;
uniform vec2 direction;
in vec2 uv;
out vec4 fragmentColor;
void main()
{
	float blur = radius/resolution;
	float hstep=direction.x;
	float vstep=irection.y;

	vec4 sum = texture2D(THtexture, vec2( uv.x - 4.0*blur*hstep , uv.y - 4.0*blur*vstep ))* 0.0162162162;
	sum += texture2D(THtexture, vec2( uv.x - 3.0*blur*hstep , uv.y - 3.0*blur*vstep ))* 0.0540540541;
	sum += texture2D(THtexture, vec2( uv.x - 2.0*blur*hstep , uv.y - 2.0*blur*vstep ))* 0.1216216216;
	sum += texture2D(THtexture, vec2( uv.x - 1.0*blur*hstep , uv.y - 1.0*blur*vstep ))* 0.1945945946;
	sum += texture2D(THtexture, vec2( uv.x , uv.y )) * 0.2270270270;
	sum += texture2D(THtexture, vec2( uv.x + 2.0*blur*hstep , uv.y + 2.0*blur*vstep ))* 0.1216216216;
	sum += texture2D(THtexture, vec2( uv.x + 3.0*blur*hstep , uv.y + 3.0*blur*vstep ))* 0.0540540541;
	sum += texture2D(THtexture, vec2( uv.x + 4.0*blur*hstep , uv.y + 4.0*blur*vstep ))* 0.0162162162;
	
	fragmentColor = vec4(sum.rgb,1.0);
}	