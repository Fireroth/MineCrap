#pragma once

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <vector>
#include "../core/camera.hpp"

extern bool inventoryOpen;
extern bool debugOpen;
extern bool pauseMenuOpen;
extern bool cursorCaptured;

class ImGuiOverlay {
public:
    ImGuiOverlay();
    ~ImGuiOverlay();

    bool init(GLFWwindow* window, GLuint textureAtlas);
    void render(float deltaTime, const Camera& camera, class World* world);

    static std::vector<const char*> blockItems;
    static std::vector<uint8_t> blockIds;
    static ImTextureID texAtlas;

    enum class PauseMenuPage {
        Main,
        Settings,
        Video,
        Controls
    };

    PauseMenuPage pauseScreenPage = PauseMenuPage::Main;

private:
    float fpsTimer;
    int frameCount;
    float fpsDisplay;
    static const float fpsRefreshInterval;
};
