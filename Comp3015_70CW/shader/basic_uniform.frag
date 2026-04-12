#version 460

const float Pi = 3.1415926535897932384626433832795;

// Inputs from vert
in vec3 FragPos;
in vec2 TexCoord;
in mat3 TBN;
in vec4 ShadowCoord;

layout (location = 0) out vec4 FragColor;

uniform int numLights;
uniform vec3 CameraPos;
uniform bool IsShadowPass;

uniform struct LightData {
    vec4 Position;  // w = 0 means Directional, w = 1 means Point
    vec3 Ld;        // Diffuse
} lights[3];        // Max 3 lights

uniform struct MaterialData {
    float Alpha;        // If object uses an image texture, the image's alpha value is prioritised (unless it is 1.0f)
    float Roughness;    // Surface roughness 0-1
    float Metallic;     // Metallic factor 0-1
    vec3 Albedo;        // Fallback albedo used if object has UseTexture = false
}Material;

uniform struct FogData {
    float Density;
    vec3 Colour;
}Fog;

uniform sampler2D DiffuseTex;   // Albedo texture
uniform sampler2D NormalTex;    // Normal map
uniform float UVScale;
uniform bool UseTexture;
uniform bool UseNormal;

uniform sampler2DShadow ShadowMap;
uniform sampler3D OffsetTex;        // Jitter texture
uniform vec3 OffsetTexSize;
uniform float Radius;               // PCF sampling radius
uniform int ShadowCastingLight;     // Index of which light is currently casting shadows
uniform float ShadowStrength;


// Controls specular highlight shape based on surface roughness
float ggxDistribution(float nDotH) {
    float alpha2 = Material.Roughness * Material.Roughness * Material.Roughness * Material.Roughness;
    float denom = (nDotH * nDotH) * (alpha2 - 1.0f) + 1.0f;

    return alpha2 / (Pi * denom * denom);
}

// Approximates shadows and masks from micro-geometry and roughness
float ggxSchlickGeometry(float nDotV) {
    float r = Material.Roughness + 1.0f;
    float k = (r * r) / 8.0f;

    return nDotV / (nDotV * (1.0f - k) + k);
}

// Combines view and light occlusion for specular attenuation
float geometrySmith(float nDotV, float nDotL) {
    return ggxSchlickGeometry(nDotV) * ggxSchlickGeometry(nDotL);
}

// Controls angle-based reflectivity
vec3 schlickFresnel(float lDotH, vec3 f0) {
    return f0 + (1.0f - f0) * pow(1.0f - lDotH, 5.0f);
}

// Cook-Torrance BRDF PBR
vec3 PBR(int light, vec3 pos, vec3 normal, vec3 albedo) {
    vec3 L;
    vec3 radiance = lights[light].Ld;

    // Directional light (sun and moon)
    if (lights[light].Position.w == 0.0f) {
        L = normalize(lights[light].Position.xyz);
    }
    // Point light (lamp)
    else {
        L = lights[light].Position.xyz - pos;

        float dist = length(L);
        L = normalize(L);

        // Attenuation (quite high because scene is small)
        radiance /= 7.0f * dist;
    }

    vec3 v = normalize(CameraPos - pos);
    vec3 h = normalize(v + L);

    float nDotL = max(dot(normal, L), 0.0f);
    float nDotV = max(dot(normal, v), 0.0f);
    float nDotH = max(dot(normal, h), 0.0f);
    float lDotH = max(dot(L, h), 0.0f);

    vec3 f0 = mix(vec3(0.04f), albedo, Material.Metallic);  // Base reflectivity
    vec3 F = schlickFresnel(lDotH, f0);

    vec3 specular = (ggxDistribution(nDotH) * F * geometrySmith(nDotV, nDotL)) / max(4.0f * nDotV * nDotL, 0.001f);
    vec3 diffuse = ((1.0f - F) * (1.0f - Material.Metallic)) * albedo / Pi;

    return (diffuse + specular) * radiance * nDotL;
}


float getShadow(vec4 shadowCoord) {
    float sum = 0.0f;
    float shadow = 1.0f;
    ivec3 offsetCoord;

    offsetCoord.xy = ivec2(gl_FragCoord.xy) % ivec2(OffsetTexSize.xy);
    int samplesDiv2 = int(OffsetTexSize.z);

    vec4 sc = shadowCoord;

    // Check if fragment is within shadow map
    if (sc.z >= 0.0f && sc.z <= sc.w) {
        sc.z -= 0.0005f * sc.w;

        // Initial PCF sampling
        for (int i = 0; i < 4; i++) {
            offsetCoord.z = i;
            vec4 offsets = texelFetch(OffsetTex, offsetCoord, 0) * Radius * shadowCoord.w;

            sc.xy = shadowCoord.xy + offsets.xy;
            sum += textureProj(ShadowMap, sc);

            sc.xy = shadowCoord.xy + offsets.zw;
            sum += textureProj(ShadowMap, sc);
        }
        
        shadow = sum / 8.0f;

        // Improve sampling near shadow edges
        if (shadow > 0.0f && shadow < 1.0f) {
            sum = 0.0f;

            for (int i = 0; i < samplesDiv2; i++) {
                offsetCoord.z = i;
                vec4 offsets = texelFetch(OffsetTex, offsetCoord, 0) * Radius * shadowCoord.w;

                sc.xy = shadowCoord.xy + offsets.xy;
                sum += textureProj(ShadowMap, sc);

                sc.xy = shadowCoord.xy + offsets.zw;
                sum += textureProj(ShadowMap, sc);
            }

            shadow = sum / float(samplesDiv2 * 2);
        }
    }

    return shadow;
}


void main() {
    vec3 baseColour;
    vec3 normal;
    float alpha;

    vec2 tiledUV = TexCoord * UVScale;

    // Check for diffuse texture
    if (UseTexture) {
        vec4 texSample = texture(DiffuseTex, tiledUV);
        // Assign texture colours to base colour
        baseColour = texSample.rgb;

        // If image has transparency, set object's transparency to that value, otherwise use material alpha
        if (texSample.a < 1.0f) {
            alpha = texSample.a;
        } else {
            alpha = Material.Alpha;
        }
    } else {
        baseColour = Material.Albedo;
        alpha = Material.Alpha;
    }

    // Check for normal texture
    if (UseNormal) {
        vec3 tangentNormal = texture(NormalTex, tiledUV).rgb;
        tangentNormal = 2.0f * tangentNormal - 1.0f;
        normal = normalize(TBN * tangentNormal);
    } else {
        normal = normalize(TBN[2]);
    }

    if (IsShadowPass) {
        // Remove low alpha pixels so transparent areas dont cast shadows
        if (alpha <= 0.4f) { discard; }

        // Return early, as shadow pass doesnt need colour information
        FragColor = vec4(0.0f);
        return;
    } else {
        // Remove zero alpha pixels to allow tree branches to be seen behind each other
        if (alpha == 0) { discard; }
    }

    float cameraDistance = length(CameraPos - FragPos);
    float fogFactor = clamp(exp(-pow(Fog.Density * cameraDistance, 2.0f)), 0.0f, 1.0f);
    
    vec3 shadingColour = vec3(0.0f);

    for (int i = 0; i < numLights; i++) {
        float shadowFactor = 1.0f;

        // Apply shadows only for currently dominant light
        if (i == ShadowCastingLight) {
            float shadow = getShadow(ShadowCoord);
            shadowFactor = mix(1.0f, shadow, ShadowStrength);
        }

        shadingColour += PBR(i, FragPos, normal, baseColour) * shadowFactor;
    }

    // Add ambient factor
    vec3 ambient = 0.05f * baseColour;
    shadingColour += ambient;

    vec3 finalColour = mix(Fog.Colour, shadingColour, fogFactor);

    FragColor = vec4(finalColour, alpha);
}
