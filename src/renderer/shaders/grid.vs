#version 330 core
layout (location = 0) in vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 worldPos;
out vec3 gridPos;

void main()
{
    worldPos = vec3(model * vec4(position, 1.0));
    gridPos = position;
    gl_Position = projection * view * model * vec4(position, 1.0);
}
