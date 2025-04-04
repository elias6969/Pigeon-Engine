#version 410 core

in vec2 TexCoords; // Interpolated texture coordinates from the vertex shader

out vec4 FragColor; // Final output color of the fragment

uniform sampler2D Basetexture; // The texture sampler

void main() {
    FragColor = texture(Basetexture, TexCoords); // Sample the texture using the UV coordinates
}

