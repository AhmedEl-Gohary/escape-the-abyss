#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

uniform sampler2D texture_diffuse1;
uniform vec3 viewPos;  // Camera position

void main() {
    // Moon light parameters
    vec3 moonColor = vec3(0.9, 0.9, 1.0);  // Brighter, slightly blue-white
    vec3 moonPos = vec3(50.0, 50.0, 50.0);  // Position of the moon in world space

    // Ambient
    float ambientStrength = 0.4;
    vec3 ambient = ambientStrength * moonColor;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 moonDir = normalize(moonPos - FragPos);
    float diff = max(dot(norm, moonDir), 0.0);
    vec3 diffuse = diff * moonColor;

    // Specular
    float specularStrength = 0.7;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-moonDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * moonColor;

    // Combine lighting
    vec3 lighting = ambient + diffuse + specular;

    // Sample texture
    vec4 texColor = texture(texture_diffuse1, TexCoord);

    // Final color with night atmosphere
    vec3 finalColor = lighting * texColor.rgb;

    // Soft night color overlay
    finalColor = mix(finalColor, finalColor * vec3(0.3, 0.3, 0.4), 0.1);

    // Add a brightness boost
    finalColor *= 1.2;

    FragColor = vec4(finalColor, texColor.a);
}