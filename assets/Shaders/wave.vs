#version 410 core

layout(location=0) in vec3 aPos;       
layout(location=1) in vec3 aNormal;    
layout(location=2) in vec2 aTexCoords; 

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vs;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Add time for animation
uniform float time;

void main() {
    // Copy position for modification
    vec3 pos = aPos;

    // Wave effect
    float waveStrength = 0.05; // height of the wave
    float waveSpeed = 2.0;     // speed of the wave
    float waveFrequency = 4.0; // how many waves
    float heightInfluence = smoothstep(0.0, 1.0, aPos.y); // more wave at top

    pos.y += sin(pos.x * waveFrequency + time * waveSpeed) * waveStrength * heightInfluence;

    vs.FragPos   = vec3(model * vec4(pos, 1.0));
    vs.Normal    = mat3(transpose(inverse(model))) * aNormal;
    vs.TexCoords = aTexCoords;
    gl_Position  = projection * view * model * vec4(pos, 1.0);
}
