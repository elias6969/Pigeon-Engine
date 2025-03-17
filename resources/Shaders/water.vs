#version 330 core

const float pi = 3.14159;

layout (location = 0) in vec3 aPos;       // Vertex position
layout (location = 1) in vec3 aNormal;    // Vertex normal
layout (location = 2) in vec2 aTexCoords; // Texture coordinates

out vec2 TexCoords;  // Pass texture coordinates to fragment shader
out vec3 Normal;     // Pass normal to fragment shader
out vec3 FragPos;    // Pass fragment position to fragment shader

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time; // Time for wave animation

uniform int numWaves;
uniform float amplitude[8];
uniform float wavelength[8];
uniform float speed[8];
uniform vec2 direction[8];

float wave(int i, float x, float z) {
    float frequency = 2 * pi / wavelength[i];
    float phase = speed[i] * frequency;
    float theta = dot(direction[i], vec2(x, z));
    return amplitude[i] * sin(theta * frequency + time * phase);
}

float waveHeight(float x, float z) {
    float height = 0.0;
    for (int i = 0; i < numWaves; ++i)
        height += wave(i, x, z);
    return height;
}

float dWavedx(int i, float x, float z) {
    float frequency = 2 * pi / wavelength[i];
    float phase = speed[i] * frequency;
    float theta = dot(direction[i], vec2(x, z));
    float A = amplitude[i] * direction[i].x * frequency;
    return A * cos(theta * frequency + time * phase);
}

float dWavedz(int i, float x, float z) {
    float frequency = 2 * pi / wavelength[i];
    float phase = speed[i] * frequency;
    float theta = dot(direction[i], vec2(x, z));
    float A = amplitude[i] * direction[i].y * frequency;
    return A * cos(theta * frequency + time * phase);
}

vec3 waveNormal(float x, float z) {
    float dx = 0.0;
    float dz = 0.0;
    for (int i = 0; i < numWaves; ++i) {
        dx += dWavedx(i, x, z);
        dz += dWavedz(i, x, z);
    }
    vec3 n = vec3(-dx, 1.0, -dz);
    return normalize(n);
}

void main() {
    vec3 modifiedPos = aPos;
    modifiedPos.y += waveHeight(aPos.x, aPos.z); // Apply wave height to y-position

    FragPos = vec3(model * vec4(modifiedPos, 1.0));
    Normal = waveNormal(aPos.x, aPos.z); // Use calculated wave normal
    TexCoords = aTexCoords;

    gl_Position = projection * view * model * vec4(modifiedPos, 1.0);
}

