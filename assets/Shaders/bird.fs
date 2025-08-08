#version 410 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D birdTexture;

void main() {
    vec4 tex = texture(birdTexture, TexCoords);
    if (tex.a < 0.1) discard; // Clean alpha edges
    if (length(tex.rgb) < 0.1) discard;
    FragColor = tex;
}
