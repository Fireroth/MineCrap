#include <windows.h>
#include <glad/glad.h>
#include "renderer/imguiOverlay.hpp"
#include "core/window.hpp"
#include "renderer/renderer.hpp"
#include "core/camera.hpp"
#include "core/input.hpp"
#include "core/options.hpp"

// "Far lands" at 4294960.0f

/*#ifdef _WIN32
extern "C" {
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif*/

//Todo: fix liquid visible face detection
//Todo: fix basically everything about liquids :}

GLFWwindow* g_currentGLFWwindow = nullptr;
GLFWwindow* getCurrentGLFWwindow() { return g_currentGLFWwindow; }


int main()
{
    int windowWidth = getOptionInt("window_width", 1280);
    int windowHeight = getOptionInt("window_height", 720);
    float aspectRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    HWND hwnd = GetConsoleWindow();
    ShowWindow(hwnd, getOptionInt("hide_console", 1) ? SW_HIDE : SW_SHOW);

    Renderer renderer;
    ImGuiOverlay ImGuiOverlay;
    BlockDB::init();
    StructureDB::init();
    
    Camera camera(
        glm::vec3(0.0f, 51.0f, 0.0f),  // Position
        glm::vec3(0.0f, 1.0f, 0.0f),   // Up vector
        0.0f,                          // Yaw
        0.0f                           // Pitch
    );

    Window window(windowWidth, windowHeight, "MineCrap");
    window.init();

    window.setFramebufferResizeCallback([&aspectRatio](int w, int h, float ar) {
        aspectRatio = ar;
    });

    GLFWwindow* glfwWindow = window.getGLFWwindow();
    g_currentGLFWwindow = glfwWindow;

    setupInputCallbacks(glfwWindow, &camera, &renderer.world);

    glfwSwapInterval(getOptionInt("vsync", 0));
    
    renderer.init();
    ImGuiOverlay.init(glfwWindow, renderer.textureAtlas);
    
    // Main game loop
    while (!window.shouldClose())
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(glfwWindow, camera, deltaTime, getSpeedMultiplier(glfwWindow));

        window.clear(0.6f, 1.0f, 1.0f, 1.0f); // Light blue background
        renderer.renderWorld(camera, aspectRatio, deltaTime, currentFrame);
        renderer.renderCrosshair(aspectRatio);
        ImGuiOverlay.render(deltaTime, camera, &renderer.world);

        window.swapBuffers();
        window.pollEvents();
    }

    return 0;
}
