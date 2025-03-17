#version 330 core

layout (location = 0) in vec3 aPos;

out float waveHeight;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

const float amplitude = 0.5;   // Wave height
const float frequency = 2.0;  // Wave frequency
const float speed = 1.0;      // Wave speed
const vec2 direction = vec2(1.0, 0.5); // Wave direction

void main()
{
    float k = frequency * 3.14159 * 2.0;      // Wave number
    float omega = speed * k;                  // Wave speed

    float wave = sin(dot(direction, aPos.xz) * k + time * omega);

    float xOffset = amplitude * cos(dot(direction, aPos.xz) * k + time * omega) * direction.x;
    float zOffset = amplitude * cos(dot(direction, aPos.xz) * k + time * omega) * direction.y;
    float yOffset = amplitude * wave;

    vec3 pos = vec3(aPos.x + xOffset, aPos.y + yOffset, aPos.z + zOffset);

    waveHeight = (yOffset + amplitude) / (amplitude * 2.0);

    gl_Position = projection * view * model * vec4(pos, 1.0);
}
