#version 410 core
in vec4 vColor;
out vec4 FragColor;

void main() {
    // Create a circular, soft edge using gl_PointCoord.
    float dist = length(gl_PointCoord - vec2(0.5));
    if(dist > 0.5)
        discard;
    FragColor = vColor;
}
