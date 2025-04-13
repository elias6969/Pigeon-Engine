#version 330 core
in vec4 vColor;
out vec4 FragColor;

void main()
{
    // Create a circular cutout based on gl_PointCoord
    float dist = length(gl_PointCoord - vec2(0.5));
    if(dist > 0.5)
        discard;  // Transparent outside the circle

    FragColor = vColor;
}
