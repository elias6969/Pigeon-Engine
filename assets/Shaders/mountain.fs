#version 410 core

in float vHeight;
out vec4 FragColor;

uniform float minHeight = 0.0;
uniform float maxHeight = 300.0;

void main()
{
    float t = clamp((vHeight - minHeight) / (maxHeight - minHeight), 0.0, 1.0);

    vec3 c0 = vec3(0.0, 0.8, 0.2);  // Green
    vec3 c1 = vec3(1.0, 1.0, 0.0);  // Yellow
    vec3 c2 = vec3(1.0, 0.0, 0.0);  // Red
    vec3 c3 = vec3(0.5, 0.2, 1.0);  // Purple
    vec3 c4 = vec3(1.0, 1.0, 1.0);  // White

    vec3 baseColor;
    if (t < 0.3) {
        baseColor = mix(c0, c1, t / 0.3);
    } else if (t < 0.5) {
        baseColor = mix(c1, c2, (t - 0.3) / 0.2);
    } else if (t < 0.7) {
        baseColor = mix(c2, c3, (t - 0.5) / 0.2);
    } else {
        baseColor = mix(c3, c4, (t - 0.7) / 0.3);
    }

    FragColor = vec4(baseColor, 1.0);
}

