#version 410 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in float aTexIndex;

out vec2 vUV;
flat out int vTexIndex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    vUV = aUV;
    vTexIndex = int(aTexIndex);
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
