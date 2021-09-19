#version 460
#extension GL_EXT_ray_tracing : require

// STRUCTS
struct Payload {
    uint seed;

    bool doesScatter;
    vec3 attenuation;
    vec3 scatterDirection;
};

struct Sphere {
    vec4 geometry;
    uint materialType;
    uint textureType;
    vec4 colors[2];
    float materialSpecificAttribute;
};


// INPUTS
layout(binding = 2) uniform Scene {
    Sphere spheres[512];
    uint sphereAmount;
} scene;

layout(location = 0) rayPayloadInEXT Payload payload;

hitAttributeEXT vec3 pointOnSphere;


// ENUMS
const uint MATERIAL_TYPE_DIFFUSE = 0;
const uint MATERIAL_TYPE_METAL = 1;
const uint MATERIAL_TYPE_REFRACTIVE = 2;

const uint TEXTURE_TYPE_SOLID = 0;
const uint TEXTURE_TYPE_CHECKERED = 1;


// METHODS
vec4 getTextureColor(const Sphere sphere);
vec3 getDiffuseScatterDirection(const Sphere sphere, const vec3 normal);
bool isVectorNearZero(const vec3 vector);
vec3 randomUnitVector();


// MAIN
void main() {
    const Sphere sphere = scene.spheres[gl_PrimitiveID];

    const vec3 normal = normalize(pointOnSphere - sphere.geometry.xyz);

    payload.doesScatter = true;
    payload.attenuation = getTextureColor(sphere).rgb;
    payload.scatterDirection = getDiffuseScatterDirection(sphere, normal);
}


// TEXTURE
vec4 getTextureColor(const Sphere sphere) {
    if (sphere.textureType == TEXTURE_TYPE_SOLID) {
        return sphere.colors[0];

    } else if (sphere.textureType == TEXTURE_TYPE_CHECKERED) {
        const float size = 6.0f;
        const float sines = sin(size * pointOnSphere.x) * sin(size * pointOnSphere.y) * sin(size * pointOnSphere.z);
        return sphere.colors[sines > 0.0f ? 0 : 1];
    }

    return sphere.colors[0];
}


// MATERIAL
vec3 getDiffuseScatterDirection(const Sphere sphere, const vec3 normal) {
    vec3 scatterDirection = normal + randomUnitVector();

    if (isVectorNearZero(scatterDirection)) {
        scatterDirection = normal;
    }

    return scatterDirection;
}


// UTILITY
bool isVectorNearZero(const vec3 vector) {
    const float s = 1e-8;
    return abs(vector.x) < s && abs(vector.y) < s && abs(vector.z) < s;
}


// RANDOM
uint randomInt() {
    payload.seed = 1664525 * payload.seed + 1013904223;
    return payload.seed;
}

float randomFloat() {
    return float(randomInt() & 0x00FFFFFFu) / float(0x01000000u);
}

float randomInInterval(const float min, const float max) {
    return randomFloat() * (max - min) + min;
}

vec3 randomVector(const float min, const float max) {
    return vec3(randomInInterval(min, max), randomInInterval(min, max), randomInInterval(min, max));
}

vec3 randomUnitVector() {
    return normalize(randomVector(-1.0f, 1.0f));
}
