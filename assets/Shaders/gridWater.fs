#version 410 core

in float waveHeight; 

out vec4 FragColor;

void main()
{
    vec3 deepBlue = vec3(0.0, 0.0, 0.5);   // Darker blue
    vec3 lightBlue = vec3(0.0, 0.5, 1.0); // Lighter blue

    // Mix colors based on wave height
    vec3 color = mix(deepBlue, lightBlue, waveHeight);
    FragColor = vec4(color, 1.0);
}
