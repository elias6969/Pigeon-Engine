#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D texture1; // Make sure the name matches setInt("texture1", 0)

void main()
{
    FragColor = texture(texture1, TexCoord);
}
