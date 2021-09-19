#version 460
#extension GL_EXT_ray_tracing : require

layout(binding = 0, rgba8) uniform image2D renderTarget;
layout(binding = 1) uniform accelerationStructureEXT accelerationStructure;

layout(location = 0) rayPayloadInEXT vec3 payload;
hitAttributeEXT vec3 pointOnSphere;

void main() {
    payload = vec3(1.0f, 0.5f, 0.0f);
}
