#version 410 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D texture_diffuse1; // Diffuse texture unit
uniform vec3 lightPos;              // Light position in world space
uniform vec3 viewPos;               // Camera position in world space

void main()
{
    // Ambient term: 10% of the texture color.
    vec3 ambient = 0.1 * texture(texture_diffuse1, TexCoords).rgb;

    // Diffuse term: compute the cosine of the angle between light and normal.
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * texture(texture_diffuse1, TexCoords).rgb;

    // Combine the contributions.
    vec3 result = ambient + diffuse;
    FragColor = vec4(result, 1.0);
}

