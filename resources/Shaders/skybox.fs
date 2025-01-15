#version 330 core
in vec3 TexCoords;
out vec4 FragColor;

void main()
{
    vec3 color = vec3(TexCoords.x, TexCoords.y, abs(TexCoords.z)); // Gradient based on position
    FragColor = vec4(color, 1.0);
}

