#version 410 core

in vec2 vUV;
flat in int vTexIndex;

out vec4 FragColor;

uniform sampler2DArray textureArray;

void main() {
    FragColor = texture(textureArray, vec3(vUV, vTexIndex));
}
