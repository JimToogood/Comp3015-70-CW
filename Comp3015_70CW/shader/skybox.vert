#version 460

// Vertex attributes
layout (location = 0) in vec3 VertexPosition;

// Outputs to frag
out vec3 TexCoords;

// Matrices
uniform mat4 MVP;


void main() {
    TexCoords = VertexPosition;

    gl_Position = MVP * vec4(VertexPosition, 1.0f);
}
