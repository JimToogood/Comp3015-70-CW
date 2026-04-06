#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

using namespace std;
using namespace glm;


class Camera {
public:
    Camera(int windowWidth, int windowHeight) :
        position(vec3(0.0f, 2.0f, 3.0f)),
        up(vec3(0.0f, 1.0f, 0.0f)),
        front(vec3(0.0f, 0.0f, -1.0f)),
        pitch(0.0f),
        yaw(-90.0f),
        lastXPos(windowWidth / 2.0f),
        lastYPos(windowHeight / 2.0f),
        isFirstFrame(true)
    {}

    void HandleKeyboard(GLFWwindow* window, float deltaTime) {
        const float speed = 3.0f * deltaTime;

        // Horizontal controls
        vec3 horizontalFront = normalize(vec3(front.x, 0.0f, front.z));
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            position += speed * horizontalFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            position -= normalize(cross(horizontalFront, up)) * speed;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            position -= speed * horizontalFront;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            position += normalize(cross(horizontalFront, up)) * speed;

        // Vertical controls
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            position += speed * up;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            position -= speed * up;
    }

    void HandleMouse(double xpos, double ypos) {
        // Initialise last x and y positions
        if (isFirstFrame) {
            lastXPos = (float)xpos;
            lastYPos = (float)ypos;
            isFirstFrame = false;
        }

        float xOffset = (float)xpos - lastXPos;
        float yOffset = lastYPos - (float)ypos;
        lastXPos = (float)xpos;
        lastYPos = (float)ypos;

        // Mouse sensitivity
        const float sensitivity = 0.04f;
        xOffset *= sensitivity;
        yOffset *= sensitivity;

        yaw += xOffset;
        pitch += yOffset;

        // Clamp pitch to avoid turning up & down too far
        pitch = clamp(pitch, -89.0f, 89.0f);

        // Calculate camera direction
        front = normalize(vec3(
            cos(radians(yaw)) * cos(radians(pitch)),
            sin(radians(pitch)),
            sin(radians(yaw)) * cos(radians(pitch))
        ));
    }

    vec3 GetPos() { return position; }
    mat4 GetView() { return lookAt(position, position + front, up); }

private:
    vec3 position;
    vec3 front;
    vec3 up;
    float yaw;
    float pitch;

    float lastXPos;
    float lastYPos;
    bool isFirstFrame;
};

#endif  // CAMERA_H
