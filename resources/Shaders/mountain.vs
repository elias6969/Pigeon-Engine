#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

// Pass height to fragment shader
out float vHeight;

// Uniforms for transformations
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Calculate world position
    vec4 worldPos = model * vec4(aPos, 1.0);

    // Store the Y (height) in vHeight
    vHeight = worldPos.y;

    // Standard MVP transform
    gl_Position = projection * view * worldPos;
}

