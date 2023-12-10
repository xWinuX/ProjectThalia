#version 450

layout(binding = 1) uniform sampler2D texSampler[12];

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in flat int textureIndex;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler[textureIndex], fragTexCoord);
}