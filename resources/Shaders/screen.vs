#version 330 core

layout (location = 0) in vec2 aPos;        // Fullscreen quad position
layout (location = 1) in vec2 aTexCoords;  // Texture coordinates

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos, 0.0, 1.0); // Position directly maps to clip-space
}

