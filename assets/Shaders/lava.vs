#version 410 core

layout(location = 0) in vec3 aPos;
layout(location = 2) in vec2 aTexCoords;

out VS_OUT {
    vec2 TexCoords;
} vs;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    vs.TexCoords = aTexCoords;

    // Basic MVP
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
