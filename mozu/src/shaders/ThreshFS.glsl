#version 460 core
out vec4 fragmentColor;
in vec2 outTextCoord;
uniform float threshold;
uniform float strength;
uniform sampler2D RTtexture;

void main()
{
    vec4 color=texture2D(RTtexture,outTextCoord);

    float brightness=dot(color.rgb,vec3(0.2126,0.7152,0.0722));
    if (brightness > threshold) fragmentColor = vec4(strength*color.rgb, 1.0 );
    else fragmentColor = vec4(0.0,0.0,0.0,1.0 );
}