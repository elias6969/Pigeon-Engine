#version 410 core
out vec4 FragColor;

in vec2 TexCoords;        // from vertex shader
uniform sampler2D Basetexture;
uniform float r, g, b;   // optional color
uniform float Alpha; // Transparency

void main()
{
    vec4 tex = texture(Basetexture, TexCoords);
    FragColor = tex * vec4(r, g, b, Alpha);
}

