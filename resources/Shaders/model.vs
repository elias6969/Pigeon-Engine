#version 330 core

layout (location = 0) in vec3 aPos;        // Vertex Position
layout (location = 1) in vec3 aNormal;     // Vertex Normal
layout (location = 2) in vec2 aTexCoords;  // Vertex Texture Coordinates

out vec3 Normal;
out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    Normal = mat3(transpose(inverse(model))) * aNormal;  
    TexCoords = aTexCoords;

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
