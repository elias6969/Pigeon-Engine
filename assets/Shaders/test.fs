#version 410 core

in vec2 TexCoords;
out vec4 FragColor; 
uniform float time;

void main() {
  vec2 centeredUV = (TexCoords - 0.5) * 2.0;
  float distance = length(centeredUV);
  float pulse = 0.25 + 0.25 * sin(time);
vec3 Colors = vec3(
    0.5 + 0.5 * sin(time),
    0.5 + 0.5 * sin(time + 1.0),
    0.5 + 0.5 * sin(time + 2.0)
);
  float circle = smoothstep(pulse + 0.02, pulse - 0.02, distance);
  if(distance <= 0.5){
  FragColor = vec4(Colors * (1.0 - circle), 1.0);
  }else{
  FragColor = vec4(0.0,0.0,0.0,1.0);
  }
}

