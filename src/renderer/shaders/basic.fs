#version 330 core

// Input from vertex shader
in vec3 fragPos;
in vec3 fragNormal;

// Output color
out vec4 fragColor;

// Material properties
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

void main()
{
    // Normalize the normal vector
    vec3 norm = normalize(fragNormal);
    
    // Calculate light direction
    vec3 lightDir = normalize(lightPos - fragPos);
    
    // Calculate view direction
    vec3 viewDir = normalize(viewPos - fragPos);
    
    // Calculate diffuse lighting
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Calculate ambient lighting
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
    
    // Calculate specular lighting
    float specularStrength = 0.5;
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;
    
    // Combine lighting components
    vec3 result = (ambient + diffuse + specular) * objectColor;
    
    fragColor = vec4(result, 1.0);
}
