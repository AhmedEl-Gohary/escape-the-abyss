#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

uniform sampler2D texture_diffuse1;
uniform vec3 viewPos;  // Camera position

// Moon Light
uniform bool isMoonOn;
uniform vec3 moonColor;
uniform vec3 moonPos;
uniform float moonIntensity;

// Light Bulb
uniform bool isLightBulbOn;
uniform vec3 lightBulbColor;
uniform vec3 lightBulbPos;
uniform float lightBulbFlicker;
uniform float lightBulbAmbientStrength;

// Flashlight
uniform bool isFlashlightOn;
uniform vec3 flashlightColor;
uniform vec3 flashlightPos;
uniform vec3 flashlightDirection;
uniform float flashlightCutOff;
uniform float flashlightOuterCutOff;

// Common light calculation function
vec3 calculatePointLight(vec3 lightPos, vec3 lightColor, float intensity, bool isLightOn) {
    if (!isLightOn) return vec3(0.0);

    // Point Light Attenuation
    float constant = 1.0;
    float linear = 0.09;
    float quadratic = 0.032;

    // Distance from fragment to light position
    float distance = length(lightPos - FragPos);

    // Calculate attenuation
    float attenuation = 1.0 / (constant + linear * distance + quadratic * distance * distance);

    // Ambient
    float ambientStrength = 0.4;
    vec3 ambient = ambientStrength * lightColor * attenuation * intensity;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * attenuation * intensity;

    // Specular
    float specularStrength = 0.7;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor * attenuation * intensity;

    return ambient + diffuse + specular;
}

// Flashlight calculation with cone effect
vec3 calculateSpotLight(vec3 lightPos, vec3 lightColor, vec3 lightDir, float cutOff, float outerCutOff, bool isLightOn) {
    if (!isLightOn) return vec3(0.0);

    vec3 fragToLight = normalize(lightPos - FragPos);

    // Calculate spotlight intensity
    float theta = dot(fragToLight, normalize(-lightDir));
    float epsilon = cutOff - outerCutOff;
    float intensity = clamp((theta - outerCutOff) / epsilon, 0.0, 1.0);

    // Point light calculations
    float constant = 1.0;
    float linear = 0.09;
    float quadratic = 0.032;

    float distance = length(lightPos - FragPos);
    float attenuation = 1.0 / (constant + linear * distance + quadratic * distance * distance);

    // Diffuse
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, fragToLight), 0.0);

    // Specular
    float specularStrength = 0.7;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-fragToLight, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

    // Combine
    vec3 ambient = 0.1 * lightColor;
    vec3 diffuse = diff * lightColor;
    vec3 specular = specularStrength * spec * lightColor;

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return ambient + diffuse + specular;
}

void main() {
    // Calculate lighting from different sources
    vec3 moonLighting = calculatePointLight(moonPos, moonColor, moonIntensity, isMoonOn);
    vec3 lightBulbLighting = calculatePointLight(lightBulbPos, lightBulbColor,
                                                 lightBulbFlicker, isLightBulbOn);
    vec3 flashlightLighting = calculateSpotLight(flashlightPos, flashlightColor,
                                                 flashlightDirection,
                                                 flashlightCutOff,
                                                 flashlightOuterCutOff,
                                                 isFlashlightOn);

    // Combine all light sources
    vec3 totalLighting = moonLighting + lightBulbLighting + flashlightLighting;

    // Sample texture
    vec4 texColor = texture(texture_diffuse1, TexCoord);

    // Final color processing
    vec3 finalColor = totalLighting * texColor.rgb;

    // Soft night color overlay
    finalColor = mix(finalColor, finalColor * vec3(0.3, 0.3, 0.4), 0.1);

    // Brightness boost
    finalColor *= 2.5;

    FragColor = vec4(finalColor, texColor.a);
}