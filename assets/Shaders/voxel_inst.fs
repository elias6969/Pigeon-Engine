#version 410 core
in vec2 TexCoord;
flat in int Layer;

uniform sampler2DArray textureArray;
uniform float Alpha;

out vec4 FragColor;

void main(){
  vec4 col = texture(textureArray, vec3(TexCoord, Layer));
  FragColor = vec4(col.rgb, col.a * Alpha);
  if(FragColor.a < 0.1) discard;
}
