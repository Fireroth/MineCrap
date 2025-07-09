#pragma once
#include <GLFW/glfw3.h>
#include <vector>
#include "../core/camera.hpp"

extern bool inventoryOpen;

class ImGuiOverlay {
public:
    ImGuiOverlay();
    ~ImGuiOverlay();

    bool init(GLFWwindow* window);
    void render(float deltaTime, const Camera& camera, class World* world);

    static std::vector<const char*> blockItems;
    static std::vector<uint8_t> blockIds;

private:
    float fpsTimer;
    int frameCount;
    float fpsDisplay;
    static const float fpsRefreshInterval;
};
