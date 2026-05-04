## 70% Coursework for the Comp3015 Games Graphics Pipelines Module in C++ OpenGL, by Jim Toogood  

---

## Assets  
All textures and models used in this project are copyright-free and licensed for free use. All rights remain with their respective creators as listed below. This project does not claim ownership of any of the third-party assets cited.  

**Models Used:**  
house.obj - https://sketchfab.com/3d-models/fantasy-house-b107e7918892434289c0166b23c6c204  
lamp.obj - https://sketchfab.com/3d-models/modular-lowpoly-medieval-environment-5bf0a1562b7e401e9e6d7758ec54d09c  
wall.obj - https://sketchfab.com/3d-models/ruined-rock-fence-75e2716c378e4a68bac3577303671921  
branch.obj & trunk.obj - https://www.turbosquid.com/3d-models/shapespark-low-poly-exterior-plants-kit-1826978  

**Texture Assets Used:**  
day_skybox.hdr - https://polyhaven.com/a/kloofendal_48d_partly_cloudy_puresky  
night_skybox.hdr - https://polyhaven.com/a/qwantani_moon_noon_puresky  
grass.jpg - https://www.poliigon.com/texture/flat-grass-texture/4585  
metal_box.png - https://freestylized.com/material/metal_plates_01/  

---

## How to run game EXE
1) Download repo  
2) Go to folder `Comp3015-70-CW-main/Comp3015_70CW/`  
3) Ensure following file structure exists there:  
```text
Comp3015_70CW/  
 ├── Project_Template.exe  
 ├── media/  
 │    ├── branch.obj  
 │    ├── branch.png  
 │    ├── grass_normal.jpg  
 │    ├── grass.jpg  
 │    ├── house_normal.png  
 │    ├── house.obj  
 │    ├── house.png  
 │    ├── lamp.obj  
 │    ├── lamp.png  
 │    ├── metal_box.obj  
 │    ├── metal_box_normal.png  
 │    ├── metal_box.png  
 │    ├── trunk.jpg  
 │    ├── trunk.obj  
 │    ├── wall_normal.png  
 │    ├── wall.obj  
 │    ├── wall.png  
 │    ├── skybox/  
 │    │    ├── day_skybox_negx.hdr  
 │    │    ├── day_skybox_negy.hdr  
 │    │    ├── day_skybox_negz.hdr  
 │    │    ├── day_skybox_posx.hdr  
 │    │    ├── day_skybox_posy.hdr  
 │    │    ├── day_skybox_posz.hdr  
 │    │    ├── night_skybox_negx.hdr  
 │    │    ├── night_skybox_negy.hdr  
 │    │    ├── night_skybox_negz.hdr  
 │    │    ├── night_skybox_posx.hdr  
 │    │    ├── night_skybox_posy.hdr  
 │    └──  └── night_skybox_posz.hdr  
 ├── shader/  
 │    ├── basic_uniform.frag  
 │    ├── basic_uniform.vert  
 │    ├── scene_fbo.frag  
 │    ├── scene_fbo.vert  
 │    ├── skybox.frag  
 │    └── skybox.vert  
 └── (any additional folders/files are unrelated to running the .exe)  
```
4) Open `Project_Template.exe` and that's it!  

*Note: If you want to build a new .exe file from inside Visual Studio, it will be built to `Comp3015_70CW/x64/Debug/`. Attempting to run the .exe file from this folder will not work, the .exe must be moved to the folder show above in order to run correctly.*  

---

## Youtube link
https://youtu.be/4sEI5BU51n4  

---

## Use of AI description
ChatGPT (OpenAI GPT-5) was used during development for:  
- Debugging and code assist  
- Assisting with GLSL code structure and optimisation  
- Helping to explain concepts such as shadow mapping and Cook–Torrance PBR  
- Repo and readme report formatting  

---

## Visual Studio and Operating System Versions
The Visual Studio and Operating System versions used to test/write the code locally were:  
- Visual Studio 2022 Version 17.14.19  
- Windows 11 (64-bit) Version 25H2  


---

## Project Overview
This project features a small outdoor scene consisting of a grassy floor, a house, a lamp post, some trees and a metal box, all surrounded by a stone wall. The primary objective of this project was to extend the 30% coursework code into a more advanced project, by utilising techniques such as Physically-Based Rendering (PBR) and shadow mapping. The main features implemented in the project are:  
- PBR using a Cook–Torrance BRDF with GGX normal distribution, Schlick Fresnel approximation, and Smith geometry (done in the `basic_uniform.frag` fragment shader)  
- Shadow mapping with Percentage-Closer Filtering (PCF) for soft shadows, including dynamically switching between sun and moon shadow casting (done in the `basic_uniform.frag` fragment shader)  
- Shadows support alpha textures, meaning objects with transparent sections (e.g. trees, lamp post) will only cast shadows with their visible (non-transparent) parts  
- Custom model (`.obj`) support with multiple scene assets  
- Multiple dynamic lights that interact and overlap, with support for both point and directional lighting  
- Distance-based fog that changes colour and intensity depending on the day/night cycle  
- Full diffuse texturing with normal maps and alpha support (has both alpha from the image texture, and custom material alpha)  
- Night and day skybox with a smooth transition between the two  
- Light animation in the form of a full day/night cycle with two lights (sun and moon) that rotate around the scene, and change colour and intensity depending on the time of day  
- Gamma correction (done in the `skybox.frag` fragment shader)  
- Framebuffer-based post processing, with a brightness/exposure modifier, and a screen vignette that changes strength depending on the day/night cycle  
- Fully controllable camera with keyboard and mouse *(see control scheme below)*  

Controls (mouse & keyboard):  
- **Fast Foward the Day/Night Cycle** with `E`  
- **Rewind the Day/Night Cycle** with `Q`  
- **Move Horizontally** left and right with `A` and `D`, forwards and backwards with `W` and `S`  
- **Move Vertically** up with `Space`, down with `LShift`  
- **Look Around** by moving the mouse  
- **Exit Program** with `Escape`  
---

## How it works
This section will only go into detail around the additions made in this 70% coursework, meaning anything that was part of the 30% will not be discussed.  

**The first major addition to this project was Shadow Mapping.**
- Firstly in `SceneBasic_Uniform.cpp`, the shadow map fbo is intialised.  
```c++
void SceneBasic_Uniform::initShadow() {
    // Create framebuffer object
    glGenFramebuffers(1, &shadowFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

    // Depth buffer texture
    glGenTextures(1, &shadowDepthTex);
    glBindTexture(GL_TEXTURE_2D, shadowDepthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowDepthTex, 0);

    // Define border
    float border[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    // Check shadow FBO initialised correctly
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        cerr << "Shadow FBO failed to initialise." << endl;
        exit(EXIT_FAILURE);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
```

- Next in `update()`, instead of having to manage multiple shadow maps, the system decides which light between the sun and moon should be casting shadows depending on the day/night cycle. The shadows are faded out then back in during the transition to avoid any visible snapping. Once the casting light has been decided, the light frustum and PV are calculated and the needed variables are passed into the GLSL shaders.  
```c++
int shadowCastingLight;
if (moonIntensity > sunIntensity) {
    shadowCastingLight = 1;
    lightFrustum.orient(moonDirection * 15.0f, vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));
} else {
    shadowCastingLight = 0;
    lightFrustum.orient(sunDirection * 12.0f, vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));
}

lightFrustum.setPerspective(100.0f, 1.0f, 1.0f, 30.0f);
lightPV = shadowBias * lightFrustum.getProjectionMatrix() * lightFrustum.getViewMatrix();

// Pass shadow variables to default shader
prog.setUniform("ShadowCastingLight", shadowCastingLight);
prog.setUniform("ShadowStrength", abs(sunIntensity - moonIntensity));
```

- After that in `render()`, the scene is first rendered from the perspective of the light that is casting shadows into a depth-only framebuffer (`shadowFBO`). This generates a shadow map that stores these values (`shadowDepthTex`). The scene is then rendered normally in the second pass. The scene renderer is provided a bool value that tells it if the current pass is the shadow pass or the regular pass, allowing it to avoid rendering unnecessary parts in the shadow pass.  
```c++
// -=-=- Pass 1: Shadow Map -=-=-
glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
glViewport(0, 0, shadowMapWidth, shadowMapHeight);
glClear(GL_DEPTH_BUFFER_BIT);

lightView = lightFrustum.getViewMatrix();
lightProjection = lightFrustum.getProjectionMatrix();

glEnable(GL_POLYGON_OFFSET_FILL);
glPolygonOffset(2.5f, 10.0f);

renderSceneObjects(true);

glDisable(GL_POLYGON_OFFSET_FILL);

// -=-=- Pass 2: Main Scene FBO -=-=-
// Render scene into FBO
glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
glViewport(0, 0, width, height);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

// Assign shadow map
glActiveTexture(GL_TEXTURE5);
glBindTexture(GL_TEXTURE_2D, shadowDepthTex);

prog.use();
prog.setUniform("ShadowMap", 5);

renderSceneObjects(false);
```

- Moving onto the fragment shader `basic_uniform.frag`, the function `getShadow()` calculates whether a fragment is in shadow by comparing its depth, from the light’s perspective, with the stored depth in the shadow map. First, the fragment position is transformed into light space (`ShadowCoord`), then multiple samples are taken from the shadow map using a jittered offset pattern to produce softer shadows. Instead of performing a single depth comparison, multiple samples are averaged using Percentage Closer Filtering to help soften shadow edges and reduces aliasing. Additionally, a second sampling pass is applied when the fragment lies near a shadow boundary, increasing the sample count around the edges.  
```glsl
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
```

- Finally, the shadows are applied in `main()`, with a check to ensure transparent areas of textures do not cast shadows.
```glsl
    if (IsShadowPass) {
        // Remove low alpha pixels so transparent areas dont cast shadows
        if (alpha <= 0.4f) { discard; }

        // Return early, as shadow pass doesnt need colour information
        FragColor = vec4(0.0f);
        return;
    }
    
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
}
```

**The second major addition to this project was PBR.**
- Firstly the old material and lighting variables used for Blinn-Phong were replaced with their PBR equivalents.  
```c++
// Lighting colour
const vec3 sunColour = vec3(0.75f, 0.71f, 0.53f) * 3.7f;
const vec3 moonColour = vec3(0.2f, 0.2f, 0.4f) * 1.5f;

...

// Materials
prog.setUniform("UseTexture", true);
prog.setUniform("UseNormal", true);
prog.setUniform("Material.Alpha", 1.0f);
prog.setUniform("Material.Roughness", 0.55f);
prog.setUniform("Material.Metallic", 0.3f);
```

- Next, the old Blinn-Phong function in `basic_uniform.frag` was replaced with a new Cook-Torrance BRDF PBR function and helper functions. The approach separates lighting into diffuse and specular parts and combines them using a microfacet surface representation. The ggx distribution function controls how rough/smooth a surface appears by shaping the specular highlights, then the Schlick Fresnel function adjusts reflectivity based on viewing angle. The geometry function approximates shadows caused by micro-geometry and roughness.
```glsl
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

    float nDotL = max(dot(normal, L), 0.001f);
    float nDotV = max(dot(normal, v), 0.001f);
    float nDotH = max(dot(normal, h), 0.0f);
    float lDotH = max(dot(L, h), 0.0f);

    vec3 f0 = mix(vec3(0.04f), albedo, Material.Metallic);  // Base reflectivity
    vec3 F = schlickFresnel(lDotH, f0);

    vec3 specular = (ggxDistribution(nDotH) * F * geometrySmith(nDotV, nDotL)) / max(4.0f * nDotV * nDotL, 0.001f);
    vec3 diffuse = ((1.0f - F) * (1.0f - Material.Metallic)) * albedo / Pi;

    return (diffuse + specular) * radiance * nDotL;
}
```

- Finally, the PBR function is utilised to calculate lighting in `main()`.  
```glsl
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
    vec3 ambient = 0.18f * baseColour;
    shadingColour += ambient;

    vec3 finalColour = mix(Fog.Colour, shadingColour, fogFactor);

    FragColor = vec4(finalColour, alpha);
```

---

## Evaluation
Overall, I believe this project meets the objectives and expectations of the coursework well, successfully demonstrating an OpenGL project that utilises multiple advanced rendering techniques in GLSL, and showcases an aestheticly pleasing scene. By combining Shadow Mapping with PBR, alongside the pre-existing features from the 30% coursework, this project has been able to produce a technically impressive codebase and a nice looking scene that could easily function as the basis for a full game engine. All these things combined help this shader program to be more technically impressive, and standout from similar OpenGL projects.  

As far as potential improvements to the project are concerned, I would've liked to have added HDR bloom to the framebuffer post processing shader to improve the look of the lighting and highlights, and better showcase the looks of the PBR lighting model, especially at midday when the sun light is the brightess or around the lamp post light at night, where the current version still feels not quite as realistic as I would like despite using PBR. Additionally, implementing some of the other Lab features into this project, for example Geometry Shaders, would've helped improve the technical impressiveness of the project.  
