#version 460 core
layout (local_size_x = 1, local_size_y = 1) in;
layout (binding = 0, rgba32f) uniform  image2D image;

void main() {
    vec4 pixel = vec4(0.0, 0.0, 0.0, 1.0);
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
 
    imageStore(image, pixelCoord, pixel);
}