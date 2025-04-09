#version 410 core

layout (location = 0) in vec3 aPos;      // Vertex position
layout (location = 1) in vec2 aTexCoord; // Texture coordinates

out vec2 TexCoords; // Pass texture coordinates to the fragment shader

// Uniforms for transformations
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0); // Transform vertex position
    TexCoords = aTexCoord; // Pass texture coordinates along
}

