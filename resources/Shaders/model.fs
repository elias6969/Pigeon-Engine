#version 330 core
uniform vec3 materialDiffuse; // Diffuse color
out vec4 FragColor;

void main() {
    FragColor = vec4(materialDiffuse, 1.0); // Render with diffuse color
}
