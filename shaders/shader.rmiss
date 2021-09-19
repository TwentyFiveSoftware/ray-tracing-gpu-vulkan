#version 460
#extension GL_EXT_ray_tracing : require

struct Payload {
    uint seed;

    bool doesScatter;
    vec3 attenuation;
    vec3 scatterDirection;
    vec3 pointOnSphere;
};

layout(location = 0) rayPayloadInEXT Payload payload;

void main() {
    payload.doesScatter = false;
    payload.attenuation = vec3(0.7f, 0.8f, 1.0f);
    payload.scatterDirection = vec3(0.0f);
    payload.pointOnSphere = vec3(0.0f);
}
