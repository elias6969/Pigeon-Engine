#version 410 core
layout (location = 0) in vec3 aPos;        // Vertex position
layout (location = 1) in vec3 aNormal;     // Vertex normal
layout (location = 2) in vec2 aTexCoords;  // Texture coordinates

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

void main()
{
    // Transform vertex position to world space.
    FragPos = vec3(model * vec4(aPos, 1.0));
    // Transform the normal properly using the inverse transpose of the model matrix.
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoords;
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
