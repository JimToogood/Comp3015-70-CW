#version 460

// Vertex attributes
layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTexCoord;
layout (location = 3) in vec4 VertexTangent;

// Outputs to frag
out vec3 FragPos;
out vec2 TexCoord;
out mat3 TBN;

// Matrices
uniform mat4 ModelMatrix;
uniform mat3 NormalMatrix;
uniform mat4 MVP;


void main() {
    // Convert to world space
    FragPos = (ModelMatrix * vec4(VertexPosition, 1.0f)).xyz;

    TexCoord = VertexTexCoord;

    // Calculate TBN for normal mapping
    vec3 N = normalize(NormalMatrix * VertexNormal);
    vec3 T = normalize(NormalMatrix * VertexTangent.xyz);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T) * VertexTangent.w;

    TBN = mat3(T, B, N);

    gl_Position = MVP * vec4(VertexPosition, 1.0f);
}
