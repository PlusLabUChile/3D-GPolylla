#version 330 core

// Input vertex attributes
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNormal;

// Uniform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Output to fragment shader
out vec3 fragPos;
out vec3 fragNormal;

void main()
{
    // Transform position to world space
    fragPos = vec3(model * vec4(vertPos, 1.0));
    
    // Transform normal to world space (using normal matrix)
    fragNormal = mat3(transpose(inverse(model))) * vertNormal;
    
    // Transform position to clip space
    gl_Position = projection * view * vec4(fragPos, 1.0);
}
