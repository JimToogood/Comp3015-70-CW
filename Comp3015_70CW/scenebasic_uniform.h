#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "helper/scene.h"
#include "helper/glslprogram.h"
#include "helper/torus.h"
#include "helper/plane.h"
#include "helper/objmesh.h"
#include "helper/texture.h"
#include "helper/skybox.h"
#include "helper/frustum.h"
#include "helper/random.h"
#include "camera.h"

using namespace std;
using namespace glm;


class SceneBasic_Uniform : public Scene {
public:
    SceneBasic_Uniform();

    void initScene(GLFWwindow* window);
    void initSceneFBO(int windowWidth, int windowHeight);
    void initShadow();
    void update(float t);
    void render();
    void renderSceneObjects(bool isShadowPass);
    void resize(int, int);

private:
    GLSLProgram prog;
    GLSLProgram skyboxProg;
    GLSLProgram sceneFBOProg;
    GLFWwindow* window;
    
    int shadowMapWidth;
    int shadowMapHeight;

    int samplesU;
    int samplesV;
    int jitterMapSize;
    float radius;

    Torus torus;
    Plane plane;
    SkyBox* skybox;
    Frustum lightFrustum;

    unique_ptr<ObjMesh> lampMesh;
    mat4 lampModel;
    mat4 torusModel;
    mat4 planeModel;

    mat4 lightPV;
    mat4 shadowBias;
    mat4 lightView;
    mat4 lightProjection;

    GLuint skyboxDayTexture;
    GLuint skyboxNightTexture;

    GLuint groundTexture;
    GLuint groundNormal;
    GLuint lampTexture;

    GLuint shadowFBO;
    GLuint shadowDepthTex;
    GLuint offsetTex;

    GLuint sceneFBO;
    GLuint sceneColourTex;
    GLuint sceneDepthTex;
    GLuint screenQuadVAO;
    GLuint screenQuadVBO;

    Camera camera;
    float deltaTime;
    float lastFrame;
    float timeOfDay;
    float dayLength;

    void setMatrices(mat4 model, bool isShadowPass);
    void compile();
    void buildJitterTex();
    float jitter();
};

#endif  // SCENEBASIC_UNIFORM_H
