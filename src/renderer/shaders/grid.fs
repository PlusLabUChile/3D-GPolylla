#version 330 core
in vec3 worldPos;
in vec3 gridPos;
out vec4 color;

uniform bool showGrid;
uniform float lineWidth;
uniform float gridSize;

void main()
{

    // Create grid pattern
    vec2 coord = gridPos.xz / gridSize;
    vec2 grid = abs(fract(coord));
    float line = min(grid.x, grid.y);
    line = min(line, 1 - max(grid.x, grid.y));

    // Fade out with distance
    float distance = length(worldPos);
    float fade = max(0.0, 1.0 - distance * 0.2);


    if (line > lineWidth)
        discard;

    // X axis
    if (abs(worldPos.y) < lineWidth && abs(worldPos.z) < lineWidth)
    {
        color = vec4(1.0, 0.0, 0.0, fade);
    }
    // Y axis
    else if (abs(worldPos.x) < lineWidth && abs(worldPos.z) < lineWidth)
    {
        color = vec4(0.0, 1.0, 0.0, fade);
    }
    // Z axis
    else if (abs(worldPos.x) < lineWidth && abs(worldPos.y) < lineWidth)
    {
        color = vec4(0.0, 0.0, 1.0, fade);
    }

    else if (showGrid) 
    {
        color = vec4(0.5, 0.5, 0.5, fade);
    }
}
