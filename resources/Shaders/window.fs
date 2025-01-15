#version 330 core

out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D texture1;

void main()
{
  vec4 texColor = texture(texture1, TexCoords);
    // Change to white if the original color is close to red
    if (texColor.r > 0.5 && texColor.g < 0.3 && texColor.b < 0.3) {
        FragColor = vec4(1.0, 1.0, 1.0, 1.0); // Output white
    } else {
        FragColor = texColor; // Otherwise, use the texture color
    }
}
