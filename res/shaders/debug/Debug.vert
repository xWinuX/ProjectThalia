#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

struct ObjectData {
    vec4 position;
};

layout(std140, set = 0, binding = 2) readonly buffer ObjectBuffer {
    ObjectData objects[1024000];
} objectBuffer;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    vec4 position = objectBuffer.objects[gl_InstanceIndex*1024 + (gl_VertexIndex / 6)].position;
    gl_Position = ubo.proj * ubo.view *  vec4(inPosition + position.xy, position.z, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}