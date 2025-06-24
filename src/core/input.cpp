#include <glad/glad.h>
#include "input.hpp"
#include "../world/block_interaction.hpp"
#include "../world/world.hpp"

static bool firstMouse = true;
static bool cursorCaptured = true;
static Camera* g_camera = nullptr;
static World* g_world = nullptr;
static float lastX;
static float lastY;
static uint8_t selectedBlockType = 1; // Default to grass

uint8_t getSelectedBlockType() {
    return selectedBlockType;
}

void setSelectedBlockType(uint8_t type) {
    selectedBlockType = type;
}

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
            placeBreakBlockOnClick(g_world, *g_camera, 'b', selectedBlockType);
        }
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        if (g_camera && g_world) {
            placeBreakBlockOnClick(g_world, *g_camera, 'p', selectedBlockType);
        }
    }
    
    // Middle mouse button: pick block type
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
        if (g_camera && g_world) {
            BlockInfo info = getLookedAtBlockInfo(g_world, *g_camera);
            if (info.valid && info.type != 0) {
                setSelectedBlockType(info.type);
            }
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
    return (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) ? 30.0f : 5.0f;
}

// Zoom state
bool getZoomState(GLFWwindow* window)
{
    return (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) ? true : false;
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
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.processKeyboard("UP", deltaTime, speedMultiplier);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.processKeyboard("DOWN", deltaTime, speedMultiplier);

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

    // Toggle wireframe mode with F key
    static bool fPressedLastFrame = false;
    static bool wireframeEnabled = false;
    bool fPressedThisFrame = glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS;
    if (fPressedThisFrame && !fPressedLastFrame) {
        wireframeEnabled = !wireframeEnabled;
        glPolygonMode(GL_FRONT_AND_BACK, wireframeEnabled ? GL_LINE : GL_FILL);
    }
    fPressedLastFrame = fPressedThisFrame;

    // Block selection with number keys 1-9
    for (int i = 1; i <= 9; ++i) {
        if (glfwGetKey(window, GLFW_KEY_1 + (i - 1)) == GLFW_PRESS) {
            selectedBlockType = static_cast<uint8_t>(i);
        }
    }
}

