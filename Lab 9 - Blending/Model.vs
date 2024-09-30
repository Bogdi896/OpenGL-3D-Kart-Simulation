#version 330 core
layout (location = 0) in vec3 aPos;   // Vertex position
layout (location = 1) in vec3 aNormal; // Normal vector - not used in this shader but usually required for mesh data
layout (location = 2) in vec2 aTexCoord; // Texture coordinates - not used in this shader

uniform mat4 model;       // Model matrix
uniform mat4 view;        // View matrix
uniform mat4 projection;  // Projection matrix

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
