#include <glad/glad.h>
#include "imguiOverlay.hpp"
#include "window.hpp"
#include "renderer.hpp"
#include "camera.hpp"
#include "input.hpp"

int windowWidth = 1280;
int windowHeight = 720;
float aspectRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
    Renderer renderer;
    ImGuiOverlay ImGuiOverlay;
    BlockDB::initialize();
    
    Camera camera(
        glm::vec3(0.0f, 47.0f, 0.0f),  // Position
        glm::vec3(0.0f, 1.0f, 0.0f),   // Up vector
        0.0f,                          // Yaw
        0.0f                           // Pitch
    );

    Window window(windowWidth, windowHeight, "MineCrap");
    if (!window.init())
        return -1;

    GLFWwindow* glfwWindow = window.getGLFWwindow();

    setupInputCallbacks(glfwWindow, &camera, &renderer.world);

    glfwSwapInterval(0); // 0 = Disable V-Sync
    
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
        renderer.renderWorld(camera, aspectRatio);
        ImGuiOverlay.render(deltaTime, camera, &renderer.world);

        window.swapBuffers();
        window.pollEvents();
    }

    return 0;
}
