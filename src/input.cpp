#include <glad/glad.h>
#include "input.hpp"
#include "block_interaction.hpp"
#include "world.hpp"

static bool firstMouse = true;
static bool cursorCaptured = true;
static Camera* g_camera = nullptr;
static World* g_world = nullptr;
static float lastX;
static float lastY;

// Mouse movement
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (!cursorCaptured) return;

    if (firstMouse)
    {
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }

    float xOffset = static_cast<float>(xpos) - lastX;
    float yOffset = lastY - static_cast<float>(ypos);

    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);

    if (g_camera)
        g_camera->processMouseMovement(xOffset, yOffset);
}

// Mouse button callback for block breaking and placing
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (!cursorCaptured) return;
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (g_camera && g_world) {
            breakBlockOnClick(g_world, *g_camera);
        }
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        if (g_camera && g_world) {
            placeBlockOnClick(g_world, *g_camera);
        }
    }
}

void setupInputCallbacks(GLFWwindow* window, Camera* camera, World* world)
{
    g_camera = camera;
    g_world = world; // Store world pointer
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    lastX = static_cast<float>(width) / 2.0f;
    lastY = static_cast<float>(height) / 2.0f;
    firstMouse = true;

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    cursorCaptured = true;
}

// Speed multiplier
float getSpeedMultiplier(GLFWwindow* window)
{
    return (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
               ? 7.0f
               : 2.0f;
}

// Keyboard movement
void processInput(GLFWwindow* window, Camera& camera, float deltaTime, float speedMultiplier)
{
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.processKeyboard("FORWARD", deltaTime, speedMultiplier);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.processKeyboard("BACKWARD", deltaTime, speedMultiplier);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.processKeyboard("LEFT", deltaTime, speedMultiplier);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.processKeyboard("RIGHT", deltaTime, speedMultiplier);


    static bool escPressedLastFrame = false;
    bool escPressedThisFrame = glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;

    if (escPressedThisFrame && !escPressedLastFrame)
    {
        cursorCaptured = !cursorCaptured;
        glfwSetInputMode(window, GLFW_CURSOR,
                         cursorCaptured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        firstMouse = true; // Reset mouse position capture
    }
    escPressedLastFrame = escPressedThisFrame;
}

