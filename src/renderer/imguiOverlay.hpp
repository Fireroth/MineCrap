#pragma once
#include <GLFW/glfw3.h>
#include "../core/camera.hpp"

class ImGuiOverlay {
public:
    ImGuiOverlay();
    ~ImGuiOverlay();

    bool init(GLFWwindow* window);
    void render(float deltaTime, const Camera& camera, class World* world);
    void shutdown();

private:
    float fpsTimer;
    int frameCount;
    float fpsDisplay;
    static const float fpsRefreshInterval;
};
