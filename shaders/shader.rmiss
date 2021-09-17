#version 460
#extension GL_EXT_ray_tracing : require

layout(location = 0) rayPayloadInEXT vec3 payload;

void main() {
    const float y = float(gl_LaunchIDEXT.y) / float(gl_LaunchSizeEXT.y);
    payload = vec3(0.0f, 0.0f, y);
}
