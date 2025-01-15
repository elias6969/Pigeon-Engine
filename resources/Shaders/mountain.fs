#version 330 core

in float vHeight;
out vec4 FragColor;

// Uniforms for dynamic height and time
uniform float minHeight = 0.0;
uniform float maxHeight = 300.0;
uniform float uTime     = 0.0;  // Pass elapsed time from CPU if you want animation

void main()
{
    // 1) Normalize the vertex height (vHeight) into [0..1]
    float t = clamp((vHeight - minHeight) / (maxHeight - minHeight), 0.0, 1.0);

    //--------------------------------------------------------------
    // 2) Define multiple color “stops” (feel free to customize!)
    //--------------------------------------------------------------
    vec3 c0 = vec3(0.0,  0.8, 0.2); // green
    vec3 c1 = vec3(1.0,  1.0, 0.0); // yellow
    vec3 c2 = vec3(1.0,  0.0, 0.0); // red
    vec3 c3 = vec3(0.5,  0.2, 1.0); // purple
    vec3 c4 = vec3(1.0,  1.0, 1.0); // white

    //--------------------------------------------------------------
    // 3) Segment t into sub-ranges and interpolate between colors
    //--------------------------------------------------------------
    // Segment layout (customize as you like!):
    //   0.0..0.3 => c0..c1 (green to yellow)
    //   0.3..0.5 => c1..c2 (yellow to red)
    //   0.5..0.7 => c2..c3 (red to purple)
    //   0.7..1.0 => c3..c4 (purple to white)
    vec3 baseColor;
    if (t < 0.3) { //if under 0.3 in height then c0 and c1
        float tt = t / 0.3;           // Rescale 0.0..0.3  -> 0.0..1.0
        baseColor = mix(c0, c1, tt);
    }
    else if (t < 0.5) { //Same here
        float tt = (t - 0.3) / 0.2;    // Rescale 0.3..0.5  -> 0.0..1.0
        baseColor = mix(c1, c2, tt);
    }
    else if (t < 0.7) {
        float tt = (t - 0.5) / 0.2;    // Rescale 0.5..0.7  -> 0.0..1.0
        baseColor = mix(c2, c3, tt);
    }
    else {
        float tt = (t - 0.7) / 0.3;    // Rescale 0.7..1.0  -> 0.0..1.0
        baseColor = mix(c3, c4, tt);
    }

    //--------------------------------------------------------------
    // 4) Add a simple time-based sine “wave” effect for variation
    //--------------------------------------------------------------
    // This wave value oscillates between -0.05 and +0.05 
    // based on elapsed time (uTime) and height (vHeight):
    float wave = sin(uTime * 4.0 + vHeight * 1.0) * 0.1;

    // Shift the final baseColor by the wave:
    vec3 finalColor = baseColor + wave;

    // Make sure the color stays within [0..1]
    finalColor = clamp(finalColor, 0.0, 1.0);

    FragColor = vec4(finalColor, 1.0);
}
