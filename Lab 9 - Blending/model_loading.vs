#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float offset;  // Uniform variable for the offset amount

void main()
{
    TexCoords = vec2(aTexCoords.x, 1.0 - aTexCoords.y);
    vec3 newPosition = aPos + aNormal * 0.05;  // Offset the position
    gl_Position = projection * view * model * vec4(newPosition, 1.0);
}
