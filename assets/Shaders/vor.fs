#version 410 core
out vec4 FragColor;

#define MAX_SEEDS 100
uniform int numPoints;
uniform vec2 seedPoints[MAX_SEEDS];
uniform vec3 seedColors[MAX_SEEDS];


in vec2 TexCoords; // if you pass from vertex shader
// or use gl_FragCoord.xy directly and normalize

uniform vec2 resolution; // screen resolution

void main()
{
    vec2 uv = gl_FragCoord.xy / resolution;
    
    float minDist = 99999.0;
    int closestIndex = 0;

    for (int i = 0; i < numPoints; i++) {
        float dist = distance(uv, seedPoints[i]);
        if (dist < minDist) {
            minDist = dist;
            closestIndex = i;
        }
    }

    // color the pixel based on region
    vec3 color = seedColors[closestIndex];
    FragColor = vec4(color, 1.0);
}
