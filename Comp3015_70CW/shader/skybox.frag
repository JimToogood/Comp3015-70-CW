#version 460

// Inputs from vert
in vec3 TexCoords;

layout (location = 0) out vec4 FragColor;

uniform samplerCube daySkyboxTex;
uniform samplerCube nightSkyboxTex;
uniform float blendFactor;

uniform vec3 sunDirection;
uniform vec3 moonDirection;
uniform vec3 sunColour;
uniform vec3 moonColour;


void main() {
    vec3 dayColour = (texture(daySkyboxTex, normalize(TexCoords)).rgb) * 1.5f;
    vec3 nightColour = texture(nightSkyboxTex, normalize(TexCoords)).rgb * 0.7f;

    vec3 skyColour = mix(nightColour, dayColour, blendFactor);

    // Calculate sun and moon visuals on skybox
    vec3 dir = normalize(TexCoords);
    skyColour += sunColour * pow(max(dot(dir, sunDirection), 0.0f), 130.0f);
    skyColour += moonColour * pow(max(dot(dir, moonDirection), 0.0f), 200.0f) * 0.4f;

    // Gamma correction
    skyColour = pow(skyColour, vec3(1.0f / 2.2f));

    FragColor = vec4(skyColour, 1.0f);
}
