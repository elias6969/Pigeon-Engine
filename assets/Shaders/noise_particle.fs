#version 330 core
in vec4 vColor;
out vec4 FragColor;

// Simple 2D hash and noise functions
float hash(vec2 p)
{
    return fract(sin(dot(p, vec2(12.9898,78.233))) * 43758.5453);
}

float noise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    vec2 u = f*f*(3.0 - 2.0*f);
    return mix(a, b, u.x)
         + (c - a)*u.y*(1.0 - u.x)
         + (d - b)*u.x*u.y;
}

void main()
{
    // Distort the point-coordinates by noise
    float scale = 10.0;       // scale up coords for more frequent noise
    float n = noise(gl_PointCoord * scale);
    vec2 offset = vec2(n * 0.1, n * 0.1);   // shift the sampling slightly
    vec2 newCoord = gl_PointCoord + offset;

    float dist = length(newCoord - vec2(0.5));
    if(dist > 0.5)
        discard;

    // Optionally fade near the edges
    float alpha = 1.0 - smoothstep(0.4, 0.5, dist);
    FragColor = vec4(vColor.rgb, vColor.a * alpha);
}
