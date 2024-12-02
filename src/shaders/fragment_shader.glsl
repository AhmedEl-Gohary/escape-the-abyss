#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

uniform sampler2D texture_diffuse1;

void main()
{
    // Basic lighting
    vec3 lightPos = vec3(0.0, 5.0, 5.0);
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    vec3 objectColor = vec3(1.0, 1.0, 1.0);

    // Ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 result = (ambient + diffuse) * objectColor;

    // Sample texture
    vec4 texColor = texture(texture_diffuse1, TexCoord);
    FragColor = vec4(result, 1.0) * texColor;
}