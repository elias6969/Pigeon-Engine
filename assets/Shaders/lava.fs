#version 410 core

in VS_OUT {
    vec2 TexCoords;
} fs;

out vec4 FragColor;

uniform float time;

// --- RANDOM/NOISE ---
float rand(vec2 co) {
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    float a = rand(i);
    float b = rand(i + vec2(1.0, 0.0));
    float c = rand(i + vec2(0.0, 1.0));
    float d = rand(i + vec2(1.0, 1.0));
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(a, b, u.x) + (c - a)* u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

void main() {
    vec2 uv = fs.TexCoords * 5.0;

    // Heat distortion
    uv += vec2(
        sin(uv.y * 10.0 + time * 3.0),
        cos(uv.x * 10.0 + time * 3.0)
    ) * 0.02;

    float n = noise(uv + time * 0.5);

    // Color gradient
    vec3 deep = vec3(0.05, 0.01, 0.0);
    vec3 red = vec3(0.6, 0.1, 0.0);
    vec3 orange = vec3(1.0, 0.3, 0.0);
    vec3 yellow = vec3(1.0, 0.9, 0.3);
    vec3 color;

    if (n < 0.4)
        color = deep;
    else if (n < 0.5)
        color = mix(deep, red, (n - 0.4) * 10.0);
    else if (n < 0.65)
        color = mix(red, orange, (n - 0.5) * 6.66);
    else
        color = mix(orange, yellow, (n - 0.65) * 6.66);

    // Bubbling & flicker
    float bubble = sin(n * 20.0 - time * 10.0) * 0.2;
    float flicker = sin(time * 12.0 + uv.x * 10.0) * 0.05;

    color += bubble * vec3(1.0, 0.3, 0.1);
    color += flicker;

    // Emissive boost
    FragColor = vec4(color * 1.5, 1.0);
}
