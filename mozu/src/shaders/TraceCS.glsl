#version 460 core
layout (local_size_x = 1, local_size_y = 1) in;
layout (binding = 0, rgba32f) uniform  image2D image;
const int samplesPerPixel = 2;
uniform mat4 invProjection;
uniform float aspectRatio;
uniform vec2 resolution;

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
 
Ray computeRay(float x, float y, vec2 pixel)
{
    float u = pixel.x / (resolution.x - 1);
    float v = pixel.y / (resolution.y - 1);

    vec3 origin = vec3(0);
    vec3 horizontal = vec3(aspectRatio * 2, 0, 0);
    vec3 vertical = vec3(0, 2, 0);
    vec3 lowerLeft = origin - horizontal / 2 - vertical / 2 - vec3(0, 0, 1);
    vec3 direction = lowerLeft + u * horizontal + v * vertical - origin;

    Ray r;

    r.origin = origin.xyz;
    r.direction = direction;

    return r;
}

bool hasHitSphere(in vec3 center, float radius, in Ray ray) {
    vec3 oc = ray.origin - center;
    float a = dot(ray.direction, ray.direction);
    float b = 2.0 * dot(oc, ray.direction);
    float c = dot(oc, oc) - radius * radius;
    float discriminant = b * b - 4 * a * c;
    return (discriminant > 0);
}

vec3 trace(in Ray ray, in Scene scene){

    if (hasHitSphere(vec3(0, 0, 1),0.5f,ray)) return vec3(0.6,0.2,0);
    vec3 unitDir = normalize(ray.direction);
    float t = 0.5 * (unitDir.y + 1.0);
    vec3 skyColor = (1.0 - t) * vec3(1.0) + t * vec3(0.3, 0.5, 1.0);
    return skyColor;

}

void main() {
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
    vec3 color = vec3(0.0);

    Scene scene;
    Ray ray = computeRay(pixelCoord.x, pixelCoord.y, gl_GlobalInvocationID.xy);
    color = trace(ray, scene);

    imageStore(image, pixelCoord, vec4(color, 0.0));

}