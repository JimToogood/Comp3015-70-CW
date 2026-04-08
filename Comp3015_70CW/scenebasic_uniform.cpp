#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include "scenebasic_uniform.h"
#include "helper/glutils.h"

using namespace std;
using namespace glm;


SceneBasic_Uniform::SceneBasic_Uniform() :
    window(nullptr),
    shadowMapWidth(512),
    shadowMapHeight(512),
    torus(0.7f, 0.3f, 100, 100),
    plane(15.0f, 15.0f, 1, 1),
    lampModel(mat4(1.0f)),
    torusModel(mat4(1.0f)),
    planeModel(mat4(1.0f)),
    shadowFBO(0),
    shadowDepthTex(0),
    skyboxDayTexture(0),
    skyboxNightTexture(0),
    groundTexture(0),
    groundNormal(0),
    lampTexture(0),
    sceneFBO(0),
    sceneColourTex(0),
    sceneDepthTex(0),
    screenQuadVAO(0),
    screenQuadVBO(0),
    camera(1280, 720),
    deltaTime(0.0f),
    lastFrame(0.0f),
    timeOfDay(0.0f),
    dayLength(30.0f)    // in seconds
{
    skybox = new SkyBox(50.0f);
}

void SceneBasic_Uniform::initScene(GLFWwindow* winIn) {
    compile();
    window = winIn;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);    // Automatically bind cursor to window & hide pointer

    initSceneFBO(1280, 720);

    // -=-=- Load models -=-=-
    // Lamp
    lampMesh = ObjMesh::load("media/lamp.obj", false);

    // -=-=- Load textures -=-=-
    // Skybox
    skyboxDayTexture = Texture::loadHdrCubeMap("media/skybox/day_skybox");
    skyboxNightTexture = Texture::loadHdrCubeMap("media/skybox/night_skybox");

    // Ground
    groundTexture = Texture::loadTexture("media/grass.jpg");
    groundNormal = Texture::loadTexture("media/grass_normal.jpg");

    // Lamp
    lampTexture = Texture::loadTexture("media/lamp.png");

    // Assign textures to shaders
    skyboxProg.use();
    skyboxProg.setUniform("daySkyboxTex", 0);
    skyboxProg.setUniform("nightSkyboxTex", 1);

    prog.use();
    prog.setUniform("DiffuseTex", 0);
    prog.setUniform("NormalTex", 1);

    // Model transforms 
    lampModel = rotate(lampModel, radians(90.0f), vec3(0.0f, 1.0f, 0.0f));
    lampModel = translate(lampModel, vec3(1.43f, 0.0f, 0.41f));

    torusModel = rotate(torusModel, radians(-35.0f), vec3(1.0f, 0.0f, 0.0f));
    torusModel = rotate(torusModel, radians(15.0f), vec3(0.0f, 1.0f, 0.0f));
    torusModel = translate(torusModel, vec3(1.0f, 1.1f, 2.0f));

    projection = mat4(1.0f);
    view = camera.GetView();

    // lights[0] = sun, lights[1] = moon
    prog.setUniform("numLights", 3);

    // Lamp light
    vec3 lampPosition = vec3(lampModel * vec4(0.0f, 1.4f, -0.7f, 1.0f));
    prog.setUniform("lights[2].Position", vec4(lampPosition, 1.0f));

    const vec3 lampColour = vec3(1.0f, 0.9f, 0.0f) * 0.09f;
    prog.setUniform("lights[2].Ld", lampColour);
    prog.setUniform("lights[2].La", lampColour * 0.25f);
    prog.setUniform("lights[2].Ls", lampColour);
}

void SceneBasic_Uniform::initSceneFBO(int windowWidth, int windowHeight) {
    // Create framebuffer object
    glGenFramebuffers(1, &sceneFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);

    // Colour buffer texture
    glGenTextures(1, &sceneColourTex);
    glBindTexture(GL_TEXTURE_2D, sceneColourTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneColourTex, 0);

    // Depth buffer texture
    glGenTextures(1, &sceneDepthTex);
    glBindTexture(GL_TEXTURE_2D, sceneDepthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, windowWidth, windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, sceneDepthTex, 0);

    // Check scene FBO initialised correctly
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        cerr << "Scene FBO failed to initialise." << endl;
        exit(EXIT_FAILURE);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Create fullscreen quad
    float quadVertices[] = {
        // Position     // UV
        -1.0f, -1.0f,   0.0f, 0.0f,
         1.0f, -1.0f,   1.0f, 0.0f,
        -1.0f,  1.0f,   0.0f, 1.0f,
         1.0f,  1.0f,   1.0f, 1.0f,
    };

    glGenVertexArrays(1, &screenQuadVAO);
    glGenBuffers(1, &screenQuadVBO);

    glBindVertexArray(screenQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, screenQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    sceneFBOProg.use();
    sceneFBOProg.setUniform("sceneTex", 0);
}

void SceneBasic_Uniform::compile() {
	try {
        sceneFBOProg.compileShader("shader/scene_fbo.vert");
        sceneFBOProg.compileShader("shader/scene_fbo.frag");
        sceneFBOProg.link();

        skyboxProg.compileShader("shader/skybox.vert");
        skyboxProg.compileShader("shader/skybox.frag");
        skyboxProg.link();

		prog.compileShader("shader/basic_uniform.vert");
		prog.compileShader("shader/basic_uniform.frag");
		prog.link();
	} catch (GLSLProgramException &e) {
		cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}
}

void SceneBasic_Uniform::update(float t) {
    // Calculate delta time
    deltaTime = t - lastFrame;
    lastFrame = t;

    timeOfDay += deltaTime;
    if (timeOfDay > dayLength) { timeOfDay -= dayLength; }

    float sunAngle = (timeOfDay / dayLength) * 2.0f * pi<float>();

    // Sun and moon orbit opposite to each other
    vec3 sunDirection = normalize(vec3(cos(sunAngle), sin(sunAngle), 0.0f));
    vec3 moonDirection = -sunDirection;

    // Define length of overlap between sun and moon at sunset/sunrise
    const float fade = 0.35f;

    float sunIntensity = clamp((sunDirection.y + fade) / (fade * 2.0f), 0.0f, 1.0f);
    float moonIntensity = clamp((moonDirection.y + fade) / (fade * 2.0f), 0.0f, 1.0f);

    // Lighting colour
    const vec3 sunColour = vec3(0.75f, 0.71f, 0.53f);
    const vec3 moonColour = vec3(0.2f, 0.2f, 0.4f);

    // Fog colour
    const vec3 fogDay = vec3(0.7f, 0.8f, 0.9f);
    const vec3 fogNight = vec3(0.07f, 0.07f, 0.1f);

    // Pass skybox variables to skybox shader
    skyboxProg.use();
    skyboxProg.setUniform("blendFactor", sunIntensity);
    skyboxProg.setUniform("sunDirection", sunDirection);
    skyboxProg.setUniform("moonDirection", moonDirection);
    skyboxProg.setUniform("sunColour", sunColour * sunIntensity);
    skyboxProg.setUniform("moonColour", moonColour * moonIntensity);

    prog.use();
    prog.setUniform("CameraPos", camera.GetPos());

    // Pass lighting and fog variables to default shader
    prog.setUniform("lights[0].Position", vec4((sunDirection * 12.0f), 0.0f));
    prog.setUniform("lights[0].Ld", sunColour * sunIntensity);
    prog.setUniform("lights[0].La", (sunColour * 0.15f) * sunIntensity);
    prog.setUniform("lights[0].Ls", (sunColour * 0.75f) * sunIntensity);

    prog.setUniform("lights[1].Position", vec4((moonDirection * 9.0f), 0.0f));
    prog.setUniform("lights[1].Ld", moonColour * moonIntensity);
    prog.setUniform("lights[1].La", (moonColour * 0.15f) * moonIntensity);
    prog.setUniform("lights[1].Ls", (moonColour * 0.55f) * moonIntensity);

    prog.setUniform("Fog.Density", mix(0.04f, 0.12f, moonIntensity));
    prog.setUniform("Fog.Colour", mix(fogDay, fogNight, moonIntensity));

    sceneFBOProg.use();
    sceneFBOProg.setUniform("vignetteStrength", mix(-0.7f, 0.05f, moonIntensity));

    // -=-=- Handle Player Input -=-=-
    // Close window on escape pressed
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    camera.HandleKeyboard(window, deltaTime);

    double x, y;
    glfwGetCursorPos(window, &x, &y);
    camera.HandleMouse(x, y);
}

void SceneBasic_Uniform::render() {
    // Render scene into FBO
    glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderSceneObjects();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Render fullscreen as quad using FBO shaders
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    sceneFBOProg.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneColourTex);

    glBindVertexArray(screenQuadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void SceneBasic_Uniform::renderSceneObjects() {
    view = camera.GetView();

    // -=-=- Skybox -=-=-
    glDepthFunc(GL_LEQUAL);
    skyboxProg.use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxDayTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxNightTexture);

    mat4 skyboxMVP = projection * mat4(mat3(view)) * mat4(1.0f);
    skyboxProg.setUniform("MVP", skyboxMVP);

    skybox->render();
    glDepthFunc(GL_LESS);

    // -=-=- Plane -=-=-
    prog.use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, groundTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, groundNormal);

    // Materials
    prog.setUniform("UVScale", 2.0f);
    prog.setUniform("UseTexture", true);
    prog.setUniform("UseNormal", true);
    prog.setUniform("Material.Shininess", 120.0f);
    prog.setUniform("Material.Alpha", 1.0f);
    prog.setUniform("Material.Ka", vec3(0.6f));
    prog.setUniform("Material.Ks", vec3(0.1f));

    // Render
    setMatrices(planeModel);
    plane.render();

    // -=-=- Torus -=-=-
    // Materials
    prog.setUniform("UVScale", 1.0f);
    prog.setUniform("UseTexture", false);
    prog.setUniform("UseNormal", false);
    prog.setUniform("Material.Shininess", 100.0f);
    prog.setUniform("Material.Kd", vec3(0.9f, 0.55f, 0.2f));
    prog.setUniform("Material.Ka", vec3(0.45f, 0.27f, 0.1f));
    prog.setUniform("Material.Ks", vec3(0.8f));

    // Render
    setMatrices(torusModel);
    torus.render();

    // -=-=- Lamp -=-=-
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, lampTexture);

    prog.setUniform("UseTexture", true);
    prog.setUniform("Material.Shininess", 80.0f);
    prog.setUniform("Material.Ka", vec3(0.7f));
    prog.setUniform("Material.Ks", vec3(0.4f));

    setMatrices(lampModel);
    lampMesh->render();
}

void SceneBasic_Uniform::resize(int w, int h) {
    glViewport(0, 0, w, h);
    width = w;
    height = h;
    projection = perspective(radians(70.0f), (float)w / h, 0.3f, 100.0f);
}

void SceneBasic_Uniform::setMatrices(mat4 model) {
    prog.setUniform("ModelMatrix", model);
    prog.setUniform("NormalMatrix", mat3(transpose(inverse(model))));
    prog.setUniform("MVP", projection * view * model);
}
