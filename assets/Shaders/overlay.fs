#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D image;
uniform vec3 tint;
uniform float alpha;

void main()
{
    vec4 tex = texture(image, TexCoords);
    float outAlpha = tex.a * alpha;

    if (outAlpha < 0.01)
        discard;

    FragColor = vec4(tex.rgb * tint, outAlpha);
}
