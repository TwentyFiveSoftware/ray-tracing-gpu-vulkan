#version 460
#extension GL_EXT_ray_tracing : require

layout(location = 0) rayPayloadInEXT vec3 payload;

void main() {
    const float t = 0.5f * (gl_WorldRayDirectionEXT.y + 1.0f);
    payload = mix(vec3(1.0f), vec3(0.5f, 0.7f, 1.0f), t);

//    payload = vec3(0.5f, 0.7f, 1.0f);
}
