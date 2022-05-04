#version 460 core
layout (local_size_x = 1, local_size_y = 1) in;
layout (binding = 0, rgba32f) uniform  image2D image;
const int samplesPerPixel = 2;
uniform mat4 invProjection;
uniform mat4 invView;
uniform float aspectRatio;
uniform vec2 resolution;
uniform float accum;
uniform float frameNo;
uniform int samples;
uniform int maxDepth;
uniform float fov;


const double pi = 3.1415926535897932385;

uint state = 0;

#define LAMBERTIAN 0
#define METALLIC 1
#define DIELECTRIC 2
#define EMISSIVE 3
#define SPHERE 0
#define XYRECT 1
#define XZRECT 2
#define YZRECT 3

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

struct Emissive
{
    vec3 emitted;
};

struct Material
{
    int type;
    vec3 albedo;
    Metal metal;
    Dielectric dielectric;
    Emissive emissive;
};

struct Sphere
{
    int matID;
    float radius;
    vec3 position;
};

struct AABBBox 
{
vec3 minimum;
vec3 maximum;
};

struct Rect
{
    vec4 box;
    float k;
    int matID;
};

struct Hittable {
int type;
Sphere sphere;
Rect rect;
};

struct Scene
{
    int matNo;
    int hitNo;
    Hittable hittables[20];
    Material materials[20];
    int typeIDs[10];
    int hitIDs[20];
};


bool refract(in vec3 v, in vec3 n, in float etaiOverEtat, out vec3 refracted) {
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

float schlick(float cosine, float refractIDX)
{
    float r0 = (1.0 - refractIDX) / (1.0 + refractIDX);
    r0 = r0 * r0;
    return r0 + (1.0 - r0) * pow((1.0 - cosine), 5);
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


bool hitXYRect( in Ray r,in Rect rect, in float tMin, in float tMax,out HitRecord hr) {

    float t = (rect.k-r.origin.z) / r.direction.z;
    if (t < tMin || t > tMax) return false;
    float x = r.origin.x + t*r.direction.x;
    float y = r.origin.y + t*r.direction.y;

    if (x < rect.box.x || x > rect.box.y || y < rect.box.z || y > rect.box.w)
        return false;
    

    hr.t = t;
    vec3 outNormal = vec3(0, 0, 1);
    hr.normal = dot(r.direction,outNormal)<0?outNormal:-outNormal;
    hr.matID = rect.matID;
    hr.position = r.origin + r.direction * hr.t;

    return true;

}

bool hitXZRect( in Ray r,in Rect rect, in float tMin, in float tMax,out HitRecord hr) {


    float t = (rect.k-r.origin.y) / r.direction.y;
    if (t < tMin || t > tMax) return false;
    float x = r.origin.x + t*r.direction.x;
    float z = r.origin.z + t*r.direction.z;
    if (x < rect.box.x || x > rect.box.y || z < rect.box.z || z > rect.box.w)
        return false;
    

    hr.t = t;
    vec3 outNormal = vec3(0, 1, 0);
    hr.normal = dot(r.direction,outNormal)<0?outNormal:-outNormal;
    hr.matID = rect.matID;
    hr.position = r.origin + r.direction * hr.t;

    return true;

}

bool hitYZRect( in Ray r,in Rect rect, in float tMin, in float tMax,out HitRecord hr) {

    float t = (rect.k-r.origin.x) / r.direction.x;
    if (t < tMin || t > tMax) return false;
    float y = r.origin.y + t*r.direction.y;
    float z = r.origin.z + t*r.direction.z;
    if (y < rect.box.x || y > rect.box.y || z < rect.box.z || z > rect.box.w)
        return false;
    

    hr.t = t;
    vec3 outNormal = vec3(1, 0, 0);
    hr.normal = dot(r.direction,outNormal)<0?outNormal:-outNormal;
    hr.matID = rect.matID;
    hr.position = r.origin + r.direction * hr.t;

    return true;

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
    HitRecord tr;
    float closest = tMax;
    bool hasHitAnything = false;
    bool hasHitSphere = false;
    int type;
 
    for (int i = 0; i <scene.hitNo; i++) {
        type = scene.hittables[i].type;
         if(type==0){
         if(hitSphere(tMin,closest,ray,scene.hittables[i].sphere,tr)) {
               hasHitAnything = true;
               closest = tr.t;
               hr = tr;
        }
         }
         if(type==1){
         if(hitXYRect(ray,scene.hittables[i].rect,tMin,closest, tr)) {
               hasHitAnything = true;
               closest = tr.t;
               hr = tr;
        }
        }
        if(type==2){
         if(hitXZRect(ray,scene.hittables[i].rect,tMin,closest, tr)) {
               hasHitAnything = true;
               closest = tr.t;
               hr = tr;
        }
        }
        if(type==3){
         if(hitYZRect(ray,scene.hittables[i].rect,tMin,closest, tr)) {
               hasHitAnything = true;
               closest = tr.t;
               hr = tr;
        }
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
    vec3 reflected = reflect(ray.direction, hr.normal);
    attenuation = vec3(1.0);
    vec3 outNormal;
    float reflectProb;
    attenuation = vec3(1.0);
    float cosTheta;
    float etaiOverEtat;


    if (dot(ray.direction, hr.normal) > 0) {
        outNormal = -hr.normal;
        etaiOverEtat = material.dielectric.refractIDX;
        cosTheta = material.dielectric.refractIDX * dot(ray.direction, hr.normal) / length(ray.direction);

    } else {
        outNormal = hr.normal;
        etaiOverEtat = 1 / material.dielectric.refractIDX;
        cosTheta = -dot(ray.direction, hr.normal) / length(ray.direction);
    }

    vec3 refracted;

    if (refract(ray.direction, outNormal, etaiOverEtat, refracted))
        reflectProb = schlick(cosTheta, material.dielectric.refractIDX);
    else
        reflectProb = 1.0;

    if (reflectProb > randomFloat(state)) {
        scattered = createRay(hr.position, normalize(reflected));

    } else {

        refract(unitDir, outNormal, etaiOverEtat, refracted);
        scattered = createRay(hr.position, normalize(refracted));

    }

    return true;
}
bool scatterEmissive(in Ray ray, HitRecord hr, Material material, out Ray scattered, out vec3 attenuation) {

    return false;
}


vec3 trace(in Ray ray, in Scene scene){
    HitRecord hr;
    Ray newRay = ray;
    vec3 attenuation = vec3(0.0);
    vec3 color = vec3(1.0);
    vec3 emitted = vec3(0.0);
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
            else if(scene.materials[hr.matID].type==EMISSIVE) {
            
                if (!scatterEmissive(newRay, hr,scene.materials[hr.matID], scattered,attenuation)) {

                    color *= scene.materials[hr.matID].emissive.emitted;
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
    if (depth < maxDepth) return +color;
    else return vec3(0.0);

}

void main() {
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
    state = gl_GlobalInvocationID.x * 1973 + gl_GlobalInvocationID.y * 9277+uint(frameNo)  * 2699 | 1;

    Scene scene;
    scene.hitNo=18;
    scene.matNo=18;

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
    scene.materials[3].metal.roughness = 0.9;

    scene.materials[4].type = DIELECTRIC;
    scene.materials[4].albedo = vec3(0.1, 0.1, 0.1);
    scene.materials[4].dielectric.refractIDX = 1.51714;

    //cornell

    scene.materials[5].type = LAMBERTIAN;
    scene.materials[5].albedo = vec3(0.35,0.95,0.36);

    scene.materials[6].type = LAMBERTIAN;
    scene.materials[6].albedo = vec3(0.88,0.88,0.88);

    scene.materials[7].type = LAMBERTIAN;
    scene.materials[7].albedo = vec3(0.93,0.29,0.93);

    //light
    scene.materials[8].type = EMISSIVE;
    scene.materials[8].emissive.emitted = vec3(1.61, 3.93, 1.85);

    scene.materials[9].type = EMISSIVE;
    scene.materials[9].emissive.emitted = vec3(3.93, 3.92, 1.61);

    scene.materials[10].type = EMISSIVE;
    scene.materials[10].emissive.emitted = vec3(3.93, 1.61, 1.69);

    //actual spheres
    scene.hittables[0].type =0;
    scene.hittables[0].sphere.radius = 3;
    scene.hittables[0].sphere.position = vec3(45.0, 3.0, 10.0);
    scene.hittables[0].sphere.matID = 5;
    scene.hitIDs[1] = 0;

    scene.hittables[1].type =0;
    scene.hittables[1].sphere.radius = 6;
    scene.hittables[1].sphere.position = vec3(45.0, 6.0, 40.0);
    scene.hittables[1].sphere.matID = 4;
    scene.hitIDs[1] = 0;

    scene.hittables[2].type =0;
    scene.hittables[2].sphere.radius = 6;
    scene.hittables[2].sphere.position = vec3(30.0, 6.0, 40.0);
    scene.hittables[2].sphere.matID = 4;
    scene.hitIDs[2] = 0;

    scene.hittables[3].type =0;
    scene.hittables[3].sphere.radius = 3;
    scene.hittables[3].sphere.position = vec3(25.0, 3.0, 20.0);
    scene.hittables[3].sphere.matID = 3;
    scene.hitIDs[3] = 0;

    scene.hittables[4].type = 0;
    scene.hittables[4].sphere.radius = 10;
    scene.hittables[4].sphere.position = vec3(15.0, 9.7, 45.0);
    scene.hittables[4].sphere.matID = 2;
    scene.hitIDs[4] = 0;

    //cornell rects
    scene.hittables[5].type =3;
    scene.hittables[5].rect.box = vec4(0, 55.5, 0, 55.5);
    scene.hittables[5].rect.k = 55.5;
    scene.hittables[5].rect.matID = 5;
    scene.hitIDs[5] = 1;

    scene.hittables[6].type =3;
    scene.hittables[6].rect.box = vec4(0, 55.5, 0, 55.5);
    scene.hittables[6].rect.k = 0;
    scene.hittables[6].rect.matID = 7;
    scene.hitIDs[6] = 1;

    scene.hittables[7].type = 2;
    scene.hittables[7].rect.box = vec4(0, 55.5, 0, 55.5);
    scene.hittables[7].rect.k = 0;
    scene.hittables[7].rect.matID = 6;
    scene.hitIDs[7] = 1;

    scene.hittables[8].type = 2;
    scene.hittables[8].rect.box = vec4(0, 55.5, 0, 55.5);
    scene.hittables[8].rect.k = 55.5;
    scene.hittables[8].rect.matID = 6;
    scene.hitIDs[8] = 1;

    scene.hittables[9].type = 1;
    scene.hittables[9].rect.box = vec4(0, 55.5, 0, 55.5);
    scene.hittables[9].rect.k = 55.5;
    scene.hittables[9].rect.matID = 6;
    scene.hitIDs[9] = 1;

    //emissive
    scene.hittables[10].type = 2;
    scene.hittables[10].rect.box = vec4(0, 56, 10, 12);
    scene.hittables[10].rect.k = 53.4;
    scene.hittables[10].rect.matID = 8;
    scene.hitIDs[10] = 1;

    scene.hittables[11].type = 2;
    scene.hittables[11].rect.box = vec4(0, 56, 25, 27);
    scene.hittables[11].rect.k = 53.4;
    scene.hittables[11].rect.matID = 9;
    scene.hitIDs[11] = 1;

    scene.hittables[12].type = 2;
    scene.hittables[12].rect.box = vec4(0, 56, 40, 42);
    scene.hittables[12].rect.k = 53.4;
    scene.hittables[12].rect.matID = 10;

    scene.hittables[13].type = 0;
    scene.hittables[13].sphere.radius = 3;
    scene.hittables[13].sphere.position = vec3(15.0, 3.2, 30.0);
    scene.hittables[13].sphere.matID = 10;
    scene.hitIDs[1] = 0;

    scene.hittables[14].type = 0;
    scene.hittables[14].sphere.radius = 3;
    scene.hittables[14].sphere.position = vec3(15.0, 3.2, 20.0);
    scene.hittables[14].sphere.matID = 9;
    scene.hitIDs[1] = 0;

    scene.hittables[15].type = 0;
    scene.hittables[15].sphere.radius = 3;
    scene.hittables[15].sphere.position = vec3(15.0, 3.2, 10.0);
    scene.hittables[15].sphere.matID = 8;
    scene.hitIDs[1] = 0;
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

    vec3 final = mix(color, previous, accum);

    imageStore(image, pixelCoord, vec4(final, 0.0));

}