#version 410 core

uniform sampler2D grassTexture;
uniform sampler2D dirtTexture;
uniform sampler2D stoneTexture;
uniform sampler2D bedrockTexture;
uniform int textureType;

in vec2 TexCoords;
out vec4 FragColor;

void main() {
    vec4 result = vec4(1.0);
    if (textureType == 0) result = texture(grassTexture, TexCoords);
    else if (textureType == 1) result = texture(dirtTexture, TexCoords);
    else if (textureType == 2) result = texture(stoneTexture, TexCoords);
    else if (textureType == 3) result = texture(bedrockTexture, TexCoords);

    FragColor = result;
}
