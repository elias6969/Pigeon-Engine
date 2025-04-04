#version 410 core

out vec4 FragColor;

// Receive the interpolated texture coordinates
in vec2 TexCoord;

// The texture sampler (bound to texture unit 0)
uniform sampler2D texture1;

void main()
{
    // Sample the texture
    vec4 texColor = texture(texture1, TexCoord);

    // Optional: If the texture has alpha < 0.1, discard for more "cutout" transparency.
    // if (texColor.a < 0.1)
    //     discard;

    // Output the sampled color
    FragColor = texColor;
}
