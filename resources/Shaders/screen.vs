#version 410 core

layout (location = 0) in vec2 aPos;        // Vertex position
layout (location = 1) in vec2 aTexCoords;  // Texture coordinates

out vec2 TexCoords; // Pass texture coordinates to fragment shader

void main()
{
    TexCoords = aTexCoords; // Pass through the texture coordinates
    gl_Position = vec4(aPos, 0.0, 1.0); // Map position to screen space
}
