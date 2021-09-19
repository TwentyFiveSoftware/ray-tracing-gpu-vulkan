#version 460
#extension GL_EXT_ray_tracing : require

layout(location = 0) rayPayloadInEXT vec3 payload;

void main() {
    payload = vec3(0.7f, 0.8f, 1.0f);
}
