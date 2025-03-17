#version 330 core

// Input texture coordinates from the vertex shader
in vec2 TexCoords;

// Output fragment color
out vec4 FragColor;

// Diffuse texture sampler (typically set to texture unit 0)
uniform sampler2D texture_diffuse1;

void main()
{
    // Sample the diffuse texture at the given UV coordinates
    FragColor = texture(texture_diffuse1, TexCoords);
}
