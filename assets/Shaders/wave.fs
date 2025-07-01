#version 410 core

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs;

out vec4 FragColor;

uniform sampler2D texture_diffuse1;  // your model’s diffuse map
uniform vec3 lightPos;               // cube’s Position
uniform vec3 lightColor;             // e.g. (1,1,1) or warm orange
uniform vec3 viewPos;                // camera.Position
uniform float shininess;             // e.g. 32.0

void main() {
    vec3 color = texture(texture_diffuse1, fs.TexCoords).rgb;

    // ambient  
    vec3 ambient = 0.1 * color * lightColor;

    // diffuse  
    vec3 N = normalize(fs.Normal);
    vec3 L = normalize(lightPos - fs.FragPos);
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * color * lightColor;

    // specular (Blinn–Phong)  
    vec3 V = normalize(viewPos - fs.FragPos);
    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), shininess);
    vec3 specular = spec * lightColor;

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
