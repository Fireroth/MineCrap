#include <glad/glad.h>
#include "renderer/imguiOverlay.hpp"
#include "core/window.hpp"
#include "renderer/renderer.hpp"
#include "core/camera.hpp"
#include "core/input.hpp"
#include "core/options.hpp"

GLFWwindow* g_currentGLFWwindow = nullptr;
GLFWwindow* getCurrentGLFWwindow() { return g_currentGLFWwindow; }

int main()
{
    int windowWidth = getOptionInt("window_width", 1280);
    int windowHeight = getOptionInt("window_height", 720);
    float aspectRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    Renderer renderer;
    ImGuiOverlay ImGuiOverlay;
    BlockDB::initialize();
    
    Camera camera(
        glm::vec3(0.0f, 51.0f, 0.0f),  // Position
        glm::vec3(0.0f, 1.0f, 0.0f),   // Up vector
        0.0f,                          // Yaw
        0.0f                           // Pitch
    );

    Window window(windowWidth, windowHeight, "MineCrap");
    if (!window.init())
        return -1;

    window.setFramebufferResizeCallback([&aspectRatio](int w, int h, float ar) {
        aspectRatio = ar;
    });

    GLFWwindow* glfwWindow = window.getGLFWwindow();
    g_currentGLFWwindow = glfwWindow;

    setupInputCallbacks(glfwWindow, &camera, &renderer.world);

    glfwSwapInterval(getOptionInt("vsync", 0));
    
    ImGuiOverlay.init(glfwWindow);
    renderer.init();
    
    // Main game loop
    while (!window.shouldClose())
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(glfwWindow, camera, deltaTime, getSpeedMultiplier(glfwWindow));

        window.clear(0.6f, 1.0f, 1.0f, 1.0f); // Light blue background
        renderer.renderWorld(camera, aspectRatio, deltaTime);
        ImGuiOverlay.render(deltaTime, camera, &renderer.world);

        window.swapBuffers();
        window.pollEvents();
    }

    return 0;
}
