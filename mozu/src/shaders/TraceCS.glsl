#version 460 core
layout (local_size_x = 1, local_size_y = 1) in;
layout (binding = 0, rgba32f) uniform  image2D image;
const int samplesPerPixel = 2;
uniform mat4 invProjection;
uniform float aspectRatio;
uniform vec2 resolution;
const double pi = 3.1415926535897932385;
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

bool hitSphere(in float tMin, in float tMax, in Ray r, in Sphere s, out HitRecord hit) {
    vec3 oc = r.origin - s.position;
    float a = dot(r.direction, r.direction);
    float b = dot(oc, r.direction);
    float c = dot(oc, oc) - s.radius * s.radius;
    float discriminant = b * b - a * c;
    if (discriminant < 0) {
        return false;
    } else {

    float root = (-b - sqrt(b * b - a * c)) / a;

        if (root > tMax || root < tMin)
        {
            root = (-b + sqrt(b * b - a * c)) / a;
            if (root > tMax || root < tMin) return false;
        }
    hit.t = root;
    hit.position = r.origin + r.direction * hit.t;
    hit.normal = normalize((hit.position - s.position) / s.radius);

    }
    return true;
}

bool hitScene(in float tMin, in float tMax, Ray ray, Scene scene, out HitRecord rec) {
    float closest = tMax;
    bool hasHitAnything = false;
    for (int i = 0; i <scene.sphereNo; i++) {
    
        if (hitSphere(tMin, closest, ray, scene.spheres[i], rec)) {
               hasHitAnything = true;
               closest = rec.t;
        }
    }
    return hasHitAnything;
}


vec3 trace(in Ray ray, in Scene scene){
    HitRecord rec;
    if(hitScene(0.001, 100000.0,ray,scene,rec)) {
    
        return 0.5 * (rec.normal + vec3(1.0));

    }
    vec3 unitDir = normalize(ray.direction);
    float t = 0.5 * (unitDir.y + 1.0);
    vec3 skyColor = vec3(0.3,0.8,1.0);
    return (1.0 - t) * vec3(1.0) + t * skyColor;

}

void main() {
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
    vec3 color = vec3(0.0);

    Scene scene;
    scene.sphereNo=2;
    scene.matNo=2;

    //groud
    scene.spheres[0].radius = 100;
    scene.spheres[0].position = vec3(0.0, -100.5, -2.0);
    scene.spheres[0].matID = 0;

    //actual sphere
    scene.spheres[1].radius = 0.5;
    scene.spheres[1].position = vec3(0.0, 0.0, -1.0);
    scene.spheres[1].matID = 0;

    Ray ray = computeRay(pixelCoord.x, pixelCoord.y, gl_GlobalInvocationID.xy);
    color = trace(ray, scene);

    imageStore(image, pixelCoord, vec4(color, 0.0));

}