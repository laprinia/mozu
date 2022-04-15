#version 460 core
layout (local_size_x = 1, local_size_y = 1) in;
layout (binding = 0, rgba32f) uniform  image2D image;
const int samplesPerPixel = 2;
uniform mat4 invProjection;

struct Ray
{
    vec3 direction;
    vec3 origin;
};

struct HitRecord
{
    float t;
    vec3 position;
    vec3 normal;
    int matID;
};

struct Metal
{
    float roughness;
};

struct Dielectric
{
    float reflection;
    float roughness;
};

struct Material
{
    int type;
    vec3 albedo;
    Metal metal;
    Dielectric dielectric;
};

struct Sphere
{
    int matID;
    float radius;
    vec3 position;
};

struct Scene
{   
    int sphereNo;
    int matNo;
    Sphere spheres[3];
    Material materials[3];
};
 
Ray computeRay(float x, float y)
{
    x = x * 2.0 - 1.0;
    y = y * 2.0 - 1.0;

    vec4 clipPosition = vec4(x, y, -1.0, 1.0);
    vec4 viewPos = invProjection * clipPosition;

    vec3 direction = vec3(invProjection * vec4(viewPos.x, viewPos.y, -1.0, 0.0));
    direction = normalize(direction);

    vec4 origin = invProjection * vec4(0.0, 0.0, 0.0, 1.0);
    origin.xyz /= origin.w;

    Ray r;

    r.origin = origin.xyz;
    r.direction = direction;

    return r;
}

vec3 trace(in Ray ray, in Scene scene){
 float t = 0.5 * (ray.direction.y + 1.0);
 vec3 skyColor = (1.0 - t) * vec3(1.0) + t * vec3(0.3, 0.5, 1.0);
 return skyColor;
}

void main() {
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
    vec3 color = vec3(0.0);

    Scene scene;
    Ray ray = computeRay(pixelCoord.x, pixelCoord.y);

    for (int i = 0; i < samplesPerPixel; i++)
    {
        color += trace(ray, scene);
     }

    color /= float(samplesPerPixel);

    vec3 previous = imageLoad(image, pixelCoord).rgb;
    
    imageStore(image, pixelCoord, vec4(color, 1.0));
}