#version 450

//layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

//layout(binding = 0) uniform UBO {
//    vec3 color;
//} ubo;

void main() {
    vec3 color = vec3(0.5);
    outColor = vec4(color, 1.0);
}