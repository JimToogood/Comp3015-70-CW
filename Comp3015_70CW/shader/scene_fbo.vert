#version 460

// Vertex attributes
layout (location = 0) in vec2 VertexPosition;
layout (location = 1) in vec2 VertexTexCoord;

// Outputs to frag
out vec2 FragUV;


void main() {
    FragUV = VertexTexCoord;

    gl_Position = vec4(VertexPosition, 0.0f, 1.0f);
}
