#version 410 core

layout (location = 0) in vec3 aPos;

out float waveHeight; // Pass this to the fragment shader

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;
uniform float amplitude, frequency, speed; //This takes care of 
void main()
{
    // Calculate the wave height based on sine waves
    float yOffset = sin(aPos.x * frequency + time * speed) * amplitude +
                    sin(aPos.z * frequency + time * speed * 1.5) * amplitude;

    vec3 pos = vec3(aPos.x, yOffset, aPos.z);

    waveHeight = (yOffset + amplitude) / (amplitude * 2.0); // Normalize to 0-1 range
    gl_Position = projection * view * model * vec4(pos, 1.0);
}

