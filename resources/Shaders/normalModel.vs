#version 330 core

// Vertex attributes: position (location = 0) and texture coordinates (location = 2)
layout(location = 0) in vec3 aPos;
layout(location = 2) in vec2 aTexCoords;

// Uniform matrices for transforming the vertex positions
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Pass texture coordinates to the fragment shader
out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
