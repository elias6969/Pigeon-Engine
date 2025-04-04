#version 410 core

out vec4 FragColor;  // Final color output
in vec2 TexCoords;   // Texture coordinates from vertex shader

uniform sampler2D screenTexture; // Screen texture input

void main()
{
    // Sample the screen texture
    vec3 color = texture(screenTexture, TexCoords).rgb;

    // Convert to grayscale
    float grayscale = dot(color, vec3(0.299, 0.587, 0.114));

    // Output the grayscale color
    FragColor = vec4(vec3(grayscale), 1.0);
}
