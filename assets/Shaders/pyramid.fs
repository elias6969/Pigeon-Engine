#version 410 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D texture_diffuse;
uniform vec3 color;

void main() {
    vec3 texColor = texture(texture_diffuse, TexCoords).rgb;
    FragColor = vec4(texColor * color, 1.0);
}

