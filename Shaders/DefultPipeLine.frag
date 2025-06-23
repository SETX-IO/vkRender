#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 TexCoord;

layout(binding = 1) uniform sampler2D texSampler;

void main() {
    outColor = texture(texSampler, TexCoord);
//    outColor = vec4(TexCoord, 0.0, 1.0);
}