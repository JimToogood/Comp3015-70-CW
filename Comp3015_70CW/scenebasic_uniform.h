#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "helper/scene.h"
#include "helper/glslprogram.h"
#include "helper/plane.h"
#include "helper/objmesh.h"
#include "helper/texture.h"
#include "helper/skybox.h"
#include "camera.h"

using namespace std;
using namespace glm;


class SceneBasic_Uniform : public Scene {
public:
    SceneBasic_Uniform();

    void initScene(GLFWwindow* window);
    void initFBO(int windowWidth, int windowHeight);
    void update(float t);
    void render();
    void renderSceneObjects();
    void resize(int, int);

private:
    GLSLProgram prog;
    GLSLProgram skyboxProg;
    GLSLProgram FBOProg;
    GLFWwindow* window;

    Plane plane;
    SkyBox* skybox;

    unique_ptr<ObjMesh> houseMesh;
    mat4 houseModel;
    mat4 houseModel2;
    unique_ptr<ObjMesh> lampMesh;
    mat4 lampModel;
    unique_ptr<ObjMesh> wallMesh;
    mat4 wallModel;
    unique_ptr<ObjMesh> branchMesh;
    unique_ptr<ObjMesh> trunkMesh;
    mat4 treeModel;
    mat4 treeModel2;
    mat4 treeModel3;
    mat4 planeModel;
    mat4 pathModel;

    GLuint skyboxDayTexture;
    GLuint skyboxNightTexture;

    GLuint groundTexture;
    GLuint groundNormal;
    GLuint houseTexture;
    GLuint houseTexture2;
    GLuint houseNormal;
    GLuint lampTexture;
    GLuint pathTexture;
    GLuint pathNormal;
    GLuint wallTexture;
    GLuint wallNormal;
    GLuint branchTexture;
    GLuint trunkTexture;

    GLuint FBO;
    GLuint FBOColourTexture;
    GLuint FBODepthTexture;
    GLuint screenQuadVAO;
    GLuint screenQuadVBO;

    Camera camera;
    float deltaTime;
    float lastFrame;
    float timeOfDay;
    float dayLength;

    void setMatrices(mat4 model);
    void compile();
};

#endif  // SCENEBASIC_UNIFORM_H
