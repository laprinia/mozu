#version 460 core
layout (local_size_x = 1, local_size_y = 1) in;
layout (binding = 0, rgba32f) uniform  image2D image;
const int samplesPerPixel = 2;
uniform mat4 invProjection;
uniform mat4 invView;
uniform float aspectRatio;
uniform vec2 resolution;
uniform int samples;
uniform int maxDepth;
uniform float fov;


const double pi = 3.1415926535897932385;

uint state = 0;

#define LAMBERTIAN 0
#define METALLIC 1
#define DIELECTRIC 2

uint random(inout uint state)
{
    uint x = state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 15;
    state = x;
    return x;
}

float randomFloat(inout uint state)
{
    return (random(state) & 0xFFFFFF) / 16777216.0f;
}

vec3 randomInUnitSphere(inout uint state)
{
    float z = randomFloat(state) * 2.0f - 1.0f;
    float t = randomFloat(state) * 2.0f * 3.1415926f;
    float r = sqrt(max(0.0, 1.0f - z * z));
    float x = r * cos(t);
    float y = r * sin(t);
    vec3 res = vec3(x, y, z);
    res *= pow(randomFloat(state), 1.0 / 3.0);
    return res;
}

vec3 randomInUnitVector(inout uint state)
{
    float z = randomFloat(state) * 2.0f - 1.0f;
    float a = randomFloat(state) * 2.0f * 3.1415926f;
    float r = sqrt(1.0f - z * z);
    float x = r * cos(a);
    float y = r * sin(a);
    return vec3(x, y, z);
}

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
    float refractIDX;
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
    Sphere spheres[10];
    Material materials[10];
};

bool refract(in vec3 v, in vec3 n, in float etaiOverEtat, out vec3 refracted)
{
    vec3 uv = normalize(v);
    float dt = dot(uv, n);
    float discriminant = 1.0 - etaiOverEtat * etaiOverEtat * (1 - dt * dt);

    if (discriminant > 0)
    {
        refracted = etaiOverEtat * (uv - n * dt) - n * sqrt(discriminant);
        return true;
    }
    else
        return false;
}
 
Ray computeRay(float x, float y, vec2 pixel)
{
   x = x * 2.0 - 1.0;
    y = y * 2.0 - 1.0;

    vec4 clip = vec4(x, y, -1.0, 1.0);
    vec4 view = invProjection * clip;

    vec3 direction = vec3(invView * vec4(view.x, view.y, -1.0, 0.0));
    direction = normalize(direction);

    vec4 origin = invView * vec4(0.0, 0.0, 0.0, 1.0);
    origin.xyz /= origin.w;

    Ray r;

    r.origin = origin.xyz;
    r.direction = direction;

    return r;
}

Ray createRay(in vec3 origin, in vec3 direction)
{
    Ray r;

    r.origin = origin;
    r.direction = direction;

    return r;
}

bool hitSphere(in float tMin, in float tMax, in Ray r, in Sphere s, out HitRecord hr) {
    vec3 oc = r.origin - s.position;
    float a = dot(r.direction, r.direction);
    float b = dot(oc, r.direction);
    float c = dot(oc, oc) - s.radius * s.radius;
    float discriminant = b * b - a * c;
    if (discriminant > 0.0)
    {
        float root = (-b - sqrt(b * b - a * c)) / a;

        if (root < tMax && root > tMin)
        {
            hr.t = root;
            hr.position = r.origin + r.direction * hr.t;
            hr.normal = normalize((hr.position - s.position) / s.radius);
            hr.matID = s.matID;

            return true;
        }

        root = (-b + sqrt(b * b - a * c)) / a;

        if (root < tMax && root > tMin)
        {
            hr.t = root;
            hr.position = r.origin + r.direction * hr.t;
            hr.normal = normalize((hr.position - s.position) / s.radius);
            hr.matID = s.matID;

            return true;
        }
    }

    return false;
}

bool hitScene(in float tMin, in float tMax, Ray ray, Scene scene, out HitRecord hr) {
    float closest = tMax;
    bool hasHitAnything = false;
    for (int i = 0; i <scene.sphereNo; i++) {
    
        if (hitSphere(tMin, closest, ray, scene.spheres[i], hr)) {
               hasHitAnything = true;
               closest = hr.t;
        }
    }
    return hasHitAnything;
}

bool scatterLambertian(in Ray ray, in HitRecord hr, Material material, out Ray scattered, out vec3 attenuation) {
    vec3 newDirection = hr.position + hr.normal + randomInUnitVector(state);
    
    scattered = createRay(hr.position, normalize(newDirection - hr.position));
    attenuation = material.albedo;
    return true;
}

bool scatterMetallic(in Ray ray, HitRecord hr, Material material, out Ray scattered, out vec3 attenuation) {
    vec3 reflected = reflect(ray.direction, hr.normal);
    float fuzziness = material.metal.roughness;
    reflected = normalize(reflected+fuzziness*randomInUnitSphere(state));
    scattered = createRay(hr.position, reflected);
    attenuation = material.albedo;

    return dot(scattered.direction,hr.normal)>0;
}

bool scatterDielectric(in Ray ray, HitRecord hr, Material material, out Ray scattered, out vec3 attenuation) {
    
    vec3 unitDir = normalize(ray.direction);
    vec3 outNormal;
    attenuation = vec3(1.0);

    float etaiOverEtat;
    if (dot(ray.direction,hr.normal) > 0) {
        outNormal = -hr.normal;
        etaiOverEtat = material.dielectric.refractIDX;
    
    } else {
        outNormal = hr.normal;
        etaiOverEtat = 1/ material.dielectric.refractIDX;
    }
    vec3 refracted;
    refract(unitDir,outNormal,etaiOverEtat,refracted);
   
    scattered = createRay(hr.position, normalize(refracted));

    return true;

}

vec3 trace(in Ray ray, in Scene scene){
    HitRecord hr;
    Ray newRay = ray;
    vec3 attenuation = vec3(0.0);
    vec3 color = vec3(1.0);
    int depth = 0;

    while (depth < maxDepth) {

        if (hitScene(0.01, 1000.0, newRay, scene, hr)) {
            Ray scattered;
            if (scene.materials[hr.matID].type==LAMBERTIAN){
                if (scatterLambertian(newRay, hr,scene.materials[hr.matID], scattered,attenuation)) {

                    color *= attenuation;
                    newRay = scattered;
                } else {

                    color *= vec3(0.0);
                    break;
                }
            }
            else if(scene.materials[hr.matID].type==METALLIC) {
            
                if (scatterMetallic(newRay, hr,scene.materials[hr.matID], scattered,attenuation)) {

                    color *= attenuation;
                    newRay = scattered;
                } else {

                    color *= vec3(0.0);
                    break;
                }
            }
            else if(scene.materials[hr.matID].type==DIELECTRIC) {
            
                if (scatterDielectric(newRay, hr,scene.materials[hr.matID], scattered,attenuation)) {

                    color *= attenuation;
                    newRay = scattered;
                } else {

                    color *= vec3(0.0);
                    break;
                }
            }
        } else {

            vec3 unitDir = normalize(ray.direction);
            float t = 0.5 * (unitDir.y + 1.0);
            vec3 skyColor = vec3(0.3, 0.8, 1.0);
            color *= (1.0 - t) * vec3(1.0) + t * skyColor;
            break;
        }

        depth++;
    }
    if (depth < maxDepth) return color;
    else return vec3(0.0);

}

void main() {
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
    state = gl_GlobalInvocationID.x * 1973 + gl_GlobalInvocationID.y * 9277  * 2699 | 1;

    Scene scene;
    scene.sphereNo=5;
    scene.matNo=5;

    //materials
    scene.materials[0].type = LAMBERTIAN;
    scene.materials[0].albedo = vec3(1.0,0.78,0.37);

    scene.materials[1].type = LAMBERTIAN;
    scene.materials[1].albedo = vec3(0.35,0.32,0.27);

    scene.materials[2].type = METALLIC;
    scene.materials[2].albedo = vec3(0.37,0.32,0.63);
    scene.materials[2].metal.roughness = 0.3;

    scene.materials[3].type = METALLIC;
    scene.materials[3].albedo = vec3(0.66,0.41,0.51);
    scene.materials[3].metal.roughness = 0.8;

    scene.materials[4].type = DIELECTRIC;
    scene.materials[4].albedo = vec3(0.52, 0.63, 0.87);
    scene.materials[4].dielectric.refractIDX = 1.51714;

    //groud
    scene.spheres[0].radius = 100;
    scene.spheres[0].position = vec3(0.0, -100.5, -2.0);
    scene.spheres[0].matID = 1;

    //actual spheres
    scene.spheres[1].radius = 0.5;
    scene.spheres[1].position = vec3(0.0, 0.01, -1.0);
    scene.spheres[1].matID = 0;

    scene.spheres[2].radius = 2;
    scene.spheres[2].position = vec3(0.0, 1.4, 2.0);
    scene.spheres[2].matID = 2;

    scene.spheres[3].radius = 1;
    scene.spheres[3].position = vec3(4.0, 0.3, 2.0);
    scene.spheres[3].matID = 3;

    scene.spheres[4].radius = 0.8;
    scene.spheres[4].position = vec3(6.0, 0.0, 2.0);
    scene.spheres[4].matID = 4;

    vec3 color = vec3(0.0);

    for (int i = 0; i < samples; i++)
    {
    vec2 altCoord = vec2(pixelCoord.x + randomFloat(state), pixelCoord.y +  + randomFloat(state));
    vec2 tCoord = altCoord / resolution;
    Ray ray = computeRay(tCoord.x, tCoord.y, gl_GlobalInvocationID.xy);
    color += trace(ray, scene);
    }
    color /=float(samples);

    vec3 previous = imageLoad(image, pixelCoord).rgb;

    vec3 final = mix(color, previous, 0);

    imageStore(image, pixelCoord, vec4(final, 0.0));

}