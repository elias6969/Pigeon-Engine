#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D text;
uniform vec3 textColor;
uniform float textAlpha;

void main()
{
    float alpha = texture(text, TexCoords).r;

    if (alpha * textAlpha < 0.01)
        discard;

    FragColor = vec4(textColor, alpha * textAlpha);
}
