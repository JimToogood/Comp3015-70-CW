#version 460

// Inputs from vert
in vec2 FragUV;

layout (location = 0) out vec4 FragColor;

uniform sampler2D sceneTex;
uniform float vignetteStrength;


void main() {
    // Sample HDR scene
    vec3 colour = texture(sceneTex, FragUV).rgb;

    // Increase brightness/exposure slightly
    colour *= 1.3f;

    // Calculate distance from centre for vignette
    vec2 centredUV = FragUV - vec2(0.5f);
    float distance = length(centredUV);

    // Apply vignette
    float vignette = smoothstep(vignetteStrength, 0.95f, 1.0f - distance);
    colour *= vignette;

    FragColor = vec4(colour, 1.0f);
}
