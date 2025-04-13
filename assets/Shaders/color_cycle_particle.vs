#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 aColor;
layout(location = 2) in float aSize;

uniform mat4 projection;
uniform mat4 view;
uniform float time;  // We'll pass this from the CPU side

out vec4 vColor;

void main()
{
    gl_Position = projection * view * vec4(aPos, 1.0);
    gl_PointSize = aSize;

    // Dynamically modulate each channel by sine(time + baseColor).
    float r = abs(sin(time + aColor.r));
    float g = abs(sin(time + aColor.g));
    float b = abs(sin(time + aColor.b));
    vColor = vec4(r, g, b, aColor.a);
}
