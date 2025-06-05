#version 450

//layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 TexCoord;

//layout(binding = 0) uniform UBO {
//    vec3 color;
//} ubo;

layout(binding = 1) uniform sampler2D texSampler;

void main() {
//    vec3 color = vec3(TexCoord, 0.0);
    outColor = texture(texSampler, TexCoord);
}