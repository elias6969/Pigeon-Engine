#version 330 core
in vec3 TexCoords;
out vec4 FragColor;

void main()
{
    vec3 color =  vec3(0.2f,0.3f,0.5f);// Gradient based on position
    FragColor = vec4(color, 1.0);
}

