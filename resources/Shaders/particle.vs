#version 330 core

layout(location = 0) in vec3 aPos;       // Position (x, y, z)
layout(location = 1) in vec2 aTexCoord;  // Texture coordinates (u, v)

// Uniforms: transformation matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Pass texture coordinates to the fragment shader
out vec2 TexCoord;

void main()
{
    // Calculate the final position of each vertex in clip space
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    
    // Pass through texture coordinates
    TexCoord = aTexCoord;
}
