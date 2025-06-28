#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;

layout(location = 2) in vec3 instancePos;

layout(location = 0) out vec2 TexCoord;

layout(binding = 0) uniform UniformObj {
    mat4 module;
    mat4 view;
    mat4 proj;
} ubo;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.module * vec4(position + instancePos, 1.0);
    TexCoord = texCoord;
}