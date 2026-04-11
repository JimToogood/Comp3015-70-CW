#version 460

// Inputs from vert
in vec3 FragPos;
in vec2 TexCoord;
in mat3 TBN;
in vec4 ShadowCoord;

layout (location = 0) out vec4 FragColor;

uniform int numLights;

uniform struct LightData {
    vec4 Position;  // w = 0 means Directional, w = 1 means Point
    vec3 Ld;        // Diffuse
    vec3 La;        // Ambient
    vec3 Ls;        // Specular
} lights[5];        // Max 5 lights

uniform struct MaterialData {
    float Shininess;
    float Alpha;    // If object uses an image texture, the image's alpha value is prioritised (unless it is 1.0f)
    vec3 Kd;        // Diffuse
    vec3 Ka;        // Ambient
    vec3 Ks;        // Specular
}Material;

uniform struct FogData {
    float Density;
    vec3 Colour;
}Fog;

uniform sampler2D DiffuseTex;
uniform sampler2D NormalTex;
uniform float UVScale;
uniform bool UseTexture;
uniform bool UseNormal;
uniform vec3 CameraPos;

uniform sampler2DShadow ShadowMap;
uniform sampler3D OffsetTex;
uniform vec3 OffsetTexSize;
uniform float Radius;
uniform int ShadowCastingLight;
uniform float ShadowStrength;


vec3 blinnPhong(int light, vec3 pos, vec3 normal, vec3 baseColour) {
    vec3 ambient = lights[light].La * baseColour;
    
    vec3 lightDir;
    float attenuation = 1.0f;

    if (lights[light].Position.w == 0.0f) {
        // Directional light (sun and moon)
        lightDir = normalize(vec3(lights[light].Position));
    } else {
        // Point light
        vec3 lightVector = vec3(lights[light].Position) - pos;
        float distance = length(lightVector);
        lightDir = normalize(lightVector);

        // Lighting affects objects less when they are further away
        attenuation = 7.0f / distance;
    }

    float sDotN = max(dot(normal, lightDir), 0.0f);
    vec3 diffuse = lights[light].Ld * baseColour * sDotN;

    vec3 specular = vec3(0.0f);

    // Only calculate specular if surface faces light
    if (sDotN > 0.0f) {
        vec3 viewDir = normalize(CameraPos - pos);
        vec3 halfVector = normalize(lightDir + viewDir);
        
        // Blinn-Phong specular calculation
        specular = lights[light].Ls * Material.Ks * pow(max(dot(normal, halfVector), 0.0f), Material.Shininess);
    }

    return (ambient + diffuse + specular) * attenuation;
}


float getShadow(vec4 shadowCoord) {
    float sum = 0.0f;
    float shadow = 1.0f;
    ivec3 offsetCoord;

    offsetCoord.xy = ivec2(gl_FragCoord.xy) % ivec2(OffsetTexSize.xy);
    int samplesDiv2 = int(OffsetTexSize.z);

    vec4 sc = shadowCoord;

    if (sc.z >= 0.0f && sc.z <= sc.w) {
        float bias = 0.0005f;
        sc.z -= bias * sc.w;

        for (int i = 0; i < 4; i++) {
            offsetCoord.z = i;
            vec4 offsets = texelFetch(OffsetTex, offsetCoord, 0) * Radius * shadowCoord.w;

            sc.xy = shadowCoord.xy + offsets.xy;
            sum += textureProj(ShadowMap, sc);

            sc.xy = shadowCoord.xy + offsets.zw;
            sum += textureProj(ShadowMap, sc);
        }
        
        shadow = sum / 8.0f;

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
        baseColour = Material.Kd;
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

    // Remove zero alpha pixels to allow tree branches to be seen behind each other
    if (alpha == 0) { discard; }

    float cameraDistance = length(CameraPos - FragPos);
    float fogFactor = clamp(exp(-pow(Fog.Density * cameraDistance, 2.0f)), 0.0f, 1.0f);
    
    vec3 shadingColour = vec3(0.0f);

    for (int i = 0; i < numLights; i++) {
        float shadowFactor = 1.0f;

        if (i == ShadowCastingLight) {
            float shadow = getShadow(ShadowCoord);
            shadowFactor = mix(1.0f, shadow, ShadowStrength);
        }

        shadingColour += blinnPhong(i, FragPos, normal, baseColour) * shadowFactor;
    }

    vec3 finalColour = mix(Fog.Colour, shadingColour, fogFactor);

    FragColor = vec4(finalColour, alpha);
}
