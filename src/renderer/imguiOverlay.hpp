#pragma once

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <vector>
#include "../core/camera.hpp"

extern bool inventoryOpen;
extern bool debugOpen;
extern bool pauseMenuOpen;
extern bool cursorCaptured;
extern bool consoleOpen;

class ImGuiOverlay {
public:
    ImGuiOverlay();
    ~ImGuiOverlay();

    bool init(GLFWwindow* window, GLuint textureAtlas);
    void render(float deltaTime, Camera& camera, class World* world);

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
    
    bool prevPauseMenuOpen = false;
    bool prevConsoleOpen = false;
    int consoleFocusDelayFrames = 0;

private:
    float fpsTimer;
    int frameCount;
    float fpsDisplay;
    static const float fpsRefreshInterval;
};
