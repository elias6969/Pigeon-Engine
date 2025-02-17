#version 330 core
out vec4 FragColor;

in vec2 TexCoord;        // from vertex shader
uniform sampler2D Basetexture;
uniform float r, g, b;   // optional color
uniform float Alpha; //Transparency

void main()
{
    vec4 tex = texture(Basetexture, TexCoord);
    FragColor = tex * vec4(r, g, b, Alpha);
}
