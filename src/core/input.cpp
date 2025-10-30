#include <glad/glad.h>
#include <array>
#include "input.hpp"
#include "../world/block_interaction.hpp"
#include "../world/world.hpp"

static bool firstMouse = true;
bool cursorCaptured = true;
bool inventoryOpen = false;
bool debugOpen = false;
bool pauseMenuOpen = false;
bool consoleOpen = false;
bool hotbarOpen = true;
static Camera* g_camera = nullptr;
static World* g_world = nullptr;
static float lastX;
static float lastY;
static uint8_t selectedBlockType = 1; // Default to grass
bool flyMode = false;
bool wireframeEnabled = false;
bool ingoreInput = false;
bool zoomedIn = false;
int selectedHotbarIndex = 0;
std::array<uint8_t, 9> hotbarBlocks = {1, 2, 3, 4, 5, 6, 7, 8, 14};

bool getZoomState(GLFWwindow*) {
    return zoomedIn;
}

uint8_t getSelectedBlockType() {
    return selectedBlockType;
}

void setSelectedBlockType(uint8_t type) {
    selectedBlockType = type;
}

void setHotbarBlock(int index, uint8_t type) {
    hotbarBlocks[index] = type;
}

// Mouse movement
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!cursorCaptured) return;

    if (firstMouse) {
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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (pauseMenuOpen) return;
    const int hotbarSize = 9;

    if (yoffset < 0) {
        selectedHotbarIndex = (selectedHotbarIndex + 1) % hotbarSize;
    } else if (yoffset > 0) {
        selectedHotbarIndex = (selectedHotbarIndex - 1 + hotbarSize) % hotbarSize;
    }

    selectedBlockType = hotbarBlocks[selectedHotbarIndex];
}

// Mouse button callback for block breaking and placing
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
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
                setHotbarBlock(selectedHotbarIndex, info.type);
                setSelectedBlockType(info.type);
            }
        }
    }
}

void setupInputCallbacks(GLFWwindow* window, Camera* camera, World* world) {
    g_camera = camera;
    g_world = world;
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    lastX = static_cast<float>(width) / 2.0f;
    lastY = static_cast<float>(height) / 2.0f;
    firstMouse = true;

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetScrollCallback(window, scroll_callback);

    if (glfwRawMouseMotionSupported()) {
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }
}

// Speed multiplier
float getSpeedMultiplier(GLFWwindow* window) {
    if (flyMode)
        return (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) ? 30.0f : 4.0f;
    else
        return (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) ? 2.5f : 1.9f;
}

// Keyboard movement
void processInput(GLFWwindow* window, Camera& camera, float deltaTime, float speedMultiplier) {
    if (g_world)
        if (flyMode)
            camera.updateVelocityFlight(deltaTime);
        else
            camera.updateVelocity(deltaTime, g_world);
    else
        camera.updateVelocity(deltaTime);

    static bool escPressedLastFrame = false;
    bool escPressedThisFrame = glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;

    static int width, height;
    glfwGetWindowSize(window, &width, &height);

    if (escPressedThisFrame && !escPressedLastFrame) {
        glfwSetCursorPos(window, width / 2.0, height / 2.0);
        if (inventoryOpen) {
            inventoryOpen = !inventoryOpen;
            cursorCaptured = !cursorCaptured;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else if (consoleOpen) {
            ingoreInput = !ingoreInput;
            consoleOpen = !consoleOpen;
            cursorCaptured = !cursorCaptured;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
            pauseMenuOpen = !pauseMenuOpen;
            consoleOpen = false;
            inventoryOpen = false;
            cursorCaptured = !cursorCaptured;
            glfwSetInputMode(window, GLFW_CURSOR,
                            cursorCaptured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
            firstMouse = true; // Reset mouse position capture
        }
    }
    escPressedLastFrame = escPressedThisFrame;

    // Toggle debug window with F3 key
    static bool f3PressedLastFrame = false;
    bool f3PressedThisFrame = glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS;
    if (f3PressedThisFrame && !f3PressedLastFrame) {
        debugOpen = !debugOpen;
    }
    f3PressedLastFrame = f3PressedThisFrame;

    // Toggle hotbar with F1 key
    static bool f1PressedLastFrame = false;
    bool f1PressedThisFrame = glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS;
    if (f1PressedThisFrame && !f1PressedLastFrame) {
        hotbarOpen = !hotbarOpen;
    }
    f1PressedLastFrame = f1PressedThisFrame;

    if (!ingoreInput) { // True if console is opened

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.processKeyboard("FORWARD", deltaTime, speedMultiplier);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.processKeyboard("BACKWARD", deltaTime, speedMultiplier);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.processKeyboard("LEFT", deltaTime, speedMultiplier);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.processKeyboard("RIGHT", deltaTime, speedMultiplier);

        if (flyMode) {
            if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
                camera.processKeyboard("DOWN", deltaTime, speedMultiplier);
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
                camera.processKeyboard("UP", deltaTime, speedMultiplier);
        } else {
            static bool spaceLast = false;
            bool spaceNow = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
            if (spaceNow && !spaceLast && g_camera) {
                g_camera->jump();
            }
            spaceLast = spaceNow;
        }

        // Zoom state
        zoomedIn = (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) ? true : false;

        // Toggle wireframe mode with G key
        static bool gPressedLastFrame = false;
        bool gPressedThisFrame = glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS;
        if (gPressedThisFrame && !gPressedLastFrame) {
            wireframeEnabled = !wireframeEnabled;
            glPolygonMode(GL_FRONT_AND_BACK, wireframeEnabled ? GL_LINE : GL_FILL);
        }
        gPressedLastFrame = gPressedThisFrame;

        // Toggle flyMode mode with F key
        static bool fPressedLastFrame = false;
        bool fPressedThisFrame = glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS;
        if (fPressedThisFrame && !fPressedLastFrame) {
            flyMode = !flyMode;
        }
        fPressedLastFrame = fPressedThisFrame;

        // Toggle chat with T key
        static bool tPressedLastFrame = false;
        bool tPressedThisFrame = glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS;
        if (tPressedThisFrame && !tPressedLastFrame) {
            if (pauseMenuOpen)
                return;
            if (!consoleOpen) {
                consoleOpen = true;
                inventoryOpen = false;

                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                ingoreInput = true;
                cursorCaptured = false;
                firstMouse = true;
            } else {
                consoleOpen = false;

                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                cursorCaptured = true;
                firstMouse = true;
            }
        }
        tPressedLastFrame = tPressedThisFrame;

        // Toggle inventory with E key
        static bool ePressedLastFrame = false;
        bool ePressedThisFrame = glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS;
        if (ePressedThisFrame && !ePressedLastFrame) {
            if (!inventoryOpen) {
                inventoryOpen = true;
                consoleOpen = false;

                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                cursorCaptured = false;
                firstMouse = true;
            } else {
                inventoryOpen = false;

                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                cursorCaptured = true;
                firstMouse = true;
            }
        }
        ePressedLastFrame = ePressedThisFrame;

        // Block selection with number keys 1-9
        for (int i = 1; i <= 9; i++) {
            if (glfwGetKey(window, GLFW_KEY_1 + (i - 1)) == GLFW_PRESS) {
                selectedBlockType = hotbarBlocks[i - 1];
                selectedHotbarIndex = i - 1;
            }
        }

    }
}

