#version 330 core
in vec4 vColor;
out vec4 FragColor;

void main()
{
    float dist = length(gl_PointCoord - vec2(0.5));
    if(dist > 0.5)
        discard;
    FragColor = vColor;
}
