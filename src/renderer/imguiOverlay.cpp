#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <sstream>
#include "../world/world.hpp"
#include "../core/camera.hpp"
#include "../world/blockDB.hpp"
#include "imguiOverlay.hpp"
#include "renderer.hpp"
#include "../world/block_interaction.hpp"
#include "../core/input.hpp"
#include "../core/options.hpp"
#include "../core/controls.hpp"

const float ImGuiOverlay::fpsRefreshInterval = 0.5f; // 500ms

std::vector<const char*> ImGuiOverlay::blockItems;
std::vector<uint8_t> ImGuiOverlay::blockIds;
ImTextureID ImGuiOverlay::texAtlas;

static std::vector<std::string> consoleLog;
static char inputBuffer[256] = "";
static bool scrollToBottom = false;

struct MenuLayout {
    ImVec2 winSize;
    float y;
    float spacing;

    MenuLayout(float contentHeight, float sp = 16.0f) : spacing(sp) {
        winSize = ImGui::GetWindowSize();
        y = (winSize.y - contentHeight) * 0.5f;
    }

    void center(float w) { ImGui::SetCursorPos(ImVec2((winSize.x - w) * 0.5f, y)); }
    void advance(float h) { y += h + spacing; }
    float centerX(float w) const { return (winSize.x - w) * 0.5f; }
};

static void drawMenuTitle(MenuLayout& layout, const char* text) {
    float w = ImGui::CalcTextSize(text).x;
    layout.center(w);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.95f));
    ImGui::TextUnformatted(text);
    ImGui::PopStyleColor();
    layout.advance(ImGui::GetTextLineHeightWithSpacing());
}

static bool drawMenuButton(MenuLayout& layout, const char* label, ImVec2 size) {
    layout.center(size.x);
    bool clicked = ImGui::Button(label, size);
    layout.advance(size.y);
    return clicked;
}

static bool drawMenuToggle(MenuLayout& layout, const char* label, bool* value, ImVec2 size) {
    char displayText[128];
    snprintf(displayText, sizeof(displayText), "%s: %s", label, *value ? "ON" : "OFF");

    if (*value) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.35f, 0.20f, 0.85f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.45f, 0.25f, 0.90f));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.30f, 0.12f, 0.12f, 0.85f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.40f, 0.18f, 0.18f, 0.90f));
    }

    layout.center(size.x);
    bool toggled = ImGui::Button(displayText, size);
    ImGui::PopStyleColor(2);

    if (toggled)
        *value = !*value;

    layout.advance(size.y);
    return toggled;
}

static bool drawMenuSliderInt(MenuLayout& layout, const char* label, const char* id, int* value, int vmin, int vmax, float sliderWidth = 300.0f, const char* valueFmt = "%d") {
    layout.center(sliderWidth);

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 10.0f));
    ImGui::PushItemWidth(sliderWidth);

    bool changed = ImGui::SliderInt(id, value, vmin, vmax, "");
    ImGui::PopItemWidth();
    ImGui::PopStyleVar();

    ImVec2 rMin = ImGui::GetItemRectMin();
    ImVec2 rMax = ImGui::GetItemRectMax();
    char displayText[128];
    char valBuf[64];
    snprintf(valBuf, sizeof(valBuf), valueFmt, *value);
    snprintf(displayText, sizeof(displayText), "%s: %s", label, valBuf);
    ImVec2 textSize = ImGui::CalcTextSize(displayText);
    ImVec2 textPos(
        rMin.x + (rMax.x - rMin.x - textSize.x) * 0.5f,
        rMin.y + (rMax.y - rMin.y - textSize.y) * 0.5f
    );
    ImGui::GetWindowDrawList()->AddText(textPos, IM_COL32(255, 255, 255, 230), displayText);

    layout.advance(rMax.y - rMin.y);
    return changed;
}

static bool drawMenuSliderFloat(MenuLayout& layout, const char* label, const char* id, float* value, float vmin, float vmax, float sliderWidth = 300.0f, const char* valueFmt = "%.1f") {
    layout.center(sliderWidth);

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 10.0f));
    ImGui::PushItemWidth(sliderWidth);
    bool changed = ImGui::SliderFloat(id, value, vmin, vmax, "");
    ImGui::PopItemWidth();
    ImGui::PopStyleVar();

    ImVec2 rMin = ImGui::GetItemRectMin();
    ImVec2 rMax = ImGui::GetItemRectMax();
    char displayText[128];
    char valBuf[64];
    snprintf(valBuf, sizeof(valBuf), valueFmt, *value);
    snprintf(displayText, sizeof(displayText), "%s: %s", label, valBuf);
    ImVec2 textSize = ImGui::CalcTextSize(displayText);
    ImVec2 textPos(
        rMin.x + (rMax.x - rMin.x - textSize.x) * 0.5f,
        rMin.y + (rMax.y - rMin.y - textSize.y) * 0.5f
    );
    ImGui::GetWindowDrawList()->AddText(textPos, IM_COL32(255, 255, 255, 230), displayText);

    layout.advance(rMax.y - rMin.y);
    return changed;
}

static void pushMenuStyle() {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.15f, 0.85f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.40f, 0.55f, 0.90f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.22f, 0.30f, 0.45f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.40f, 0.40f, 0.45f, 0.55f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.55f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.15f, 0.15f, 0.20f, 0.65f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.15f, 0.15f, 0.20f, 0.70f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.85f, 0.85f, 0.85f, 0.70f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(1.00f, 1.00f, 1.00f, 0.85f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 2.0f);
}

static void popMenuStyle() {
    ImGui::PopStyleVar(4);
    ImGui::PopStyleColor(9);
}


ImGuiOverlay::ImGuiOverlay()
    : fpsTimer(0.0f), frameCount(0), fpsDisplay(0.0f) {
    prevPauseMenuOpen = false;
    prevConsoleOpen = false;
    consoleFocusDelayFrames = 0;
}

ImGuiOverlay::~ImGuiOverlay() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

bool ImGuiOverlay::init(GLFWwindow* window, GLuint textureAtlas) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    for (uint8_t id = 1; id <= 254; id++) {
        const auto* info = BlockDB::getBlockInfo(id);
        if (info) {
            blockItems.push_back(info->name.c_str());
            blockIds.push_back(id);
        }
    }

    // Keep tab insertion order consistent across platforms
    for (size_t i = 0; i < blockItems.size(); i++) {
        const auto* blockInfo = BlockDB::getBlockInfo(blockIds[i]);
        auto& indices = tabMap[blockInfo->tabName];
        if (indices.empty())
            tabOrder.push_back(blockInfo->tabName);
        indices.push_back(i);
    }

    texAtlas = (ImTextureID)(intptr_t)textureAtlas;

    // Generate 3D block preview textures for inventory andhotbar
    BlockPreviewRenderer::init(textureAtlas);
    BlockPreviewRenderer::generatePreviews();

    ImFont* Font = io.Fonts->AddFontFromFileTTF("./Font.ttf", 25.0f);

    return true;
}

void ImGuiOverlay::render(float deltaTime, Camera& camera, World* world, Renderer* renderer) {
    frameCount++;
    fpsTimer += deltaTime;

    if (fpsTimer >= fpsRefreshInterval) {
        fpsDisplay = frameCount / fpsTimer;
        frameCount = 0;
        fpsTimer = 0.0f;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // ---------------- Pause menu screen ----------------
    if (pauseMenuOpen) {
        if (!prevPauseMenuOpen)
            pauseScreenPage = PauseMenuPage::Main;

        debugOpen = false;
        inventoryOpen = false;

        ImGuiIO& io = ImGui::GetIO();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.7f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

        ImGui::Begin("Pause Menu",
                    nullptr,
                    ImGuiWindowFlags_NoTitleBar | 
                    ImGuiWindowFlags_NoResize | 
                    ImGuiWindowFlags_NoMove | 
                    ImGuiWindowFlags_NoCollapse |
                    ImGuiWindowFlags_NoBringToFrontOnFocus | 
                    ImGuiWindowFlags_NoNavFocus);
        
        const ImVec2 buttonSize(220, 45);
        const float spacing = 16.0f;
        const float titleH = ImGui::GetTextLineHeightWithSpacing();
        const float sliderH = ImGui::GetFrameHeight();

        pushMenuStyle();

        switch (pauseScreenPage) {
            case PauseMenuPage::Main: {
                float totalH = titleH + spacing + 3 * buttonSize.y + 2 * spacing;
                MenuLayout layout(totalH, spacing);

                drawMenuTitle(layout, "Game Paused");

                if (drawMenuButton(layout, "Resume", buttonSize)) {
                    pauseMenuOpen = false;
                    cursorCaptured = true;
                    glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                }

                if (drawMenuButton(layout, "Settings", buttonSize))
                    pauseScreenPage = PauseMenuPage::Settings;

                if (drawMenuButton(layout, "Quit", buttonSize))
                    glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
            } break;

            case PauseMenuPage::Settings: {
                float totalH = titleH + spacing + 3 * buttonSize.y + 2 * spacing;
                MenuLayout layout(totalH, spacing);

                drawMenuTitle(layout, "Settings");

                if (drawMenuButton(layout, "Video", buttonSize))
                    pauseScreenPage = PauseMenuPage::Video;

                if (drawMenuButton(layout, "Controls", buttonSize))
                    pauseScreenPage = PauseMenuPage::ControlsCustomize;

                if (drawMenuButton(layout, "Back", buttonSize))
                    pauseScreenPage = PauseMenuPage::Main;
            } break;

            case PauseMenuPage::Video: {
                float sliderHeight = buttonSize.y;
                float totalH = titleH + spacing + 3 * (sliderHeight + spacing) + buttonSize.y + spacing + buttonSize.y;
                MenuLayout layout(totalH, spacing);

                drawMenuTitle(layout, "Video Settings");

                int renderDistance = getOptionInt("render_distance", 7);
                if (drawMenuSliderInt(layout, "Render Distance", "##RenderDistance", &renderDistance, 1, 32)) {
                    saveOption("render_distance", renderDistance, "options.txt");
                    world->updateChunksAroundPlayer(camera.getPosition(), renderDistance, true);
                }

                float fov = getOptionFloat("fov", 70.0f);
                if (drawMenuSliderFloat(layout, "Field of View", "##FOV", &fov, 10.0f, 110.0f)) {
                    saveOption("fov", static_cast<int>(fov), "options.txt");
                }

                int maxFps = getOptionInt("max_fps", 60);
                int sliderVal = (maxFps == 0) ? 250 : maxFps;
                const char* fpsFormat = (sliderVal >= 250) ? "Unlimited" : "%d";
                if (drawMenuSliderInt(layout, "Max FPS", "##MaxFPS", &sliderVal, 10, 250, 300.0f, fpsFormat)) {
                    saveOption("max_fps", (sliderVal >= 250) ? 0 : sliderVal, "options.txt");
                }

                if (drawMenuToggle(layout, "Fog", &renderer->fogEnabled, ImVec2(300, buttonSize.y))) {
                    saveOption("fog", renderer->fogEnabled ? 1 : 0, "options.txt");
                }

                if (drawMenuButton(layout, "Back", buttonSize))
                    pauseScreenPage = PauseMenuPage::Settings;
            } break;

            case PauseMenuPage::ControlsCustomize: {
                ImGuiIO& io = ImGui::GetIO();
                ImVec2 winSize = ImGui::GetWindowSize();
                float controlsWidth = 600.0f;
                float controlsHeight = io.DisplaySize.y * 0.75f;
                float bottomBtnH = 40.0f;
                float totalH = controlsHeight + spacing + bottomBtnH;
                MenuLayout layout(totalH, spacing);

                layout.center(controlsWidth);
                ImGui::BeginChild("ControlsCustomize", ImVec2(controlsWidth, controlsHeight), true);
                ImGui::Text("Customize Controls");
                ImGui::Separator();

                static int selectedControlIndex = -1;
                static bool waitingForKey = false;
                static int pendingKeyControl = -1;

                ImGui::Text("Click on a control to rebind it, then press the new key");
                ImGui::Spacing();

                // Helper lambda to display control row
                auto displayControlRow = [&](const char* label, int& controlKey, int controlIndex) {
                    bool isSelected = (selectedControlIndex == controlIndex && waitingForKey);
                    
                    if (isSelected) {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.5f, 0.0f, 0.9f));
                    }

                    std::string displayText = std::string(label) + ": " + getKeyName(controlKey);
                    if (ImGui::Button(displayText.c_str(), ImVec2(controlsWidth - 20, 30))) {
                        selectedControlIndex = controlIndex;
                        waitingForKey = true;
                        pendingKeyControl = controlIndex;
                    }

                    if (isSelected) {
                        ImGui::PopStyleColor();
                        ImGui::Indent(10.0f);
                        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Press a key...");
                        ImGui::Unindent(10.0f);
                    }

                    if (isSelected) {
                        GLFWwindow* window = glfwGetCurrentContext();
                        for (int key = 0; key < GLFW_KEY_LAST; key++) {
                            if (glfwGetKey(window, key) == GLFW_PRESS) {
                                if (key == GLFW_KEY_ESCAPE) {
                                    waitingForKey = false;
                                    selectedControlIndex = -1;
                                } else {
                                    controlKey = key;
                                    waitingForKey = false;
                                    selectedControlIndex = -1;
                                }
                                break;
                            }
                        }
                    }
                };

                displayControlRow("Move Forward", g_controls.moveForward, 0);
                displayControlRow("Move Backward", g_controls.moveBackward, 1);
                displayControlRow("Move Left", g_controls.moveLeft, 2);
                displayControlRow("Move Right", g_controls.moveRight, 3);
                displayControlRow("Jump/Up", g_controls.jumpUp, 4);
                displayControlRow("Down", g_controls.crouchDown, 5);
                displayControlRow("Sprint", g_controls.sprint, 6);
                displayControlRow("Toggle Fly Mode", g_controls.toggleFlyMode, 7);
                displayControlRow("Toggle Wireframe", g_controls.toggleWireframe, 8);
                displayControlRow("Open Inventory", g_controls.openInventory, 9);
                displayControlRow("Open Console", g_controls.openConsole, 10);
                displayControlRow("Toggle Hotbar", g_controls.toggleHotbar, 11);
                displayControlRow("Toggle Debug", g_controls.toggleDebug, 12);
                displayControlRow("Zoom", g_controls.zoom, 13);

                ImGui::EndChild();
                layout.advance(controlsHeight);

                float btnW = 140.0f;
                float btnGap = 8.0f;
                float totalBtnW = 3 * btnW + 2 * btnGap;
                layout.center(totalBtnW);

                if (ImGui::Button("Load Defaults", ImVec2(btnW, bottomBtnH))) {
                    initializeDefaultControls();
                }
                ImGui::SameLine(0.0f, btnGap);
                if (ImGui::Button("Save & Back", ImVec2(btnW, bottomBtnH))) {
                    saveControlsToFile("controls.txt");
                    pauseScreenPage = PauseMenuPage::Settings;
                }
                ImGui::SameLine(0.0f, btnGap);
                if (ImGui::Button("Cancel", ImVec2(btnW, bottomBtnH))) {
                    pauseScreenPage = PauseMenuPage::Settings;
                }
            } break;
        }

        popMenuStyle();

        ImGui::End();

        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor();
    }

    prevPauseMenuOpen = pauseMenuOpen;

    // ---------------- Debug window ----------------
    if (debugOpen) {
        ImGui::SetNextWindowSize(ImVec2(450, 0)); // Width, Height
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        
        glm::dvec3 pos = camera.getPositionDouble();
        glm::vec3 front = camera.getFront();
        glm::vec3 up = camera.getUp();
        BlockInfo blockInfo = getLookedAtBlockInfo(world, camera);
        float camYaw = camera.getYaw();
        float camPitch = camera.getPitch();
        bool grounded = camera.isGrounded();

        uint8_t selectedBlockType = getSelectedBlockType();
        glm::dvec3 velocity = camera.getVelocity();
        float speedHor = static_cast<float>(glm::length(glm::dvec2(velocity.x, velocity.z)));
        float speedVer = static_cast<float>(glm::length(velocity.y));
        float speed = static_cast<float>(glm::length(velocity));
        
        float eyeHeight = camera.getEyeHeight();
        glm::dvec3 feetPos = glm::dvec3(pos.x, pos.y - eyeHeight, pos.z);
        int chunkX = static_cast<int>(std::floor(feetPos.x / 16.0f));
        int chunkZ = static_cast<int>(std::floor(feetPos.z / 16.0f));

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 0.5f));
        ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(1.0f, 1.0f, 1.0f, 0.7f));

        ImGui::Begin("Debug",
                    nullptr,
                    ImGuiWindowFlags_NoTitleBar | 
                    ImGuiWindowFlags_NoMove | 
                    ImGuiWindowFlags_NoResize | 
                    ImGuiWindowFlags_NoCollapse);
        ImGui::Text("FPS: %.1f", fpsDisplay);
        ImGui::Text("Pos: %.2f / %.2f / %.2f", feetPos.x-0.5, feetPos.y, feetPos.z-0.5);
        ImGui::Text("Delta Time: %.2f ms", deltaTime*1000);
        ImGui::Text("Chunk: %d, %d", chunkX, chunkZ);
        ImGui::Separator();
        ImGui::Text("Camera -> Yaw: %.2f", camYaw);
        ImGui::Text("Camera -> Pitch: %.2f", camPitch);
        ImGui::Text("Camera -> Front: %.2f, %.2f, %.2f", front.x, front.y, front.z);
        ImGui::Text("Camera -> Up: %.2f, %.2f, %.2f", up.x, up.y, up.z);
        ImGui::Text("Camera -> Speed (Horizontal): %.2f", speedHor);
        ImGui::Text("Camera -> Speed (Vertical): %.2f", speedVer);
        ImGui::Text("Camera -> Speed (Total): %.2f", speed);
        ImGui::Text("Camera -> Grounded: %s", grounded ? "True" : "False");
        ImGui::Separator();
        if (blockInfo.valid) {
            const auto* info = BlockDB::getBlockInfo(blockInfo.type);
            ImGui::Text("Block -> name: %s", info->name.c_str());
            ImGui::Text("Block -> ID: %d", blockInfo.type);
            ImGui::Text("Block -> position: %d / %d / %d", blockInfo.worldPos.x, blockInfo.worldPos.y, blockInfo.worldPos.z);
            ImGui::Text("Block -> transparent: %s", info->transparent ? "True" : "False");
            ImGui::Text("Block -> renderFacesInBetween: %s", info->renderFacesInBetween ? "True" : "False");
            ImGui::Text("Block -> model name: %s", info->modelName.c_str());
        } else {
            ImGui::Text("Block -> name: Air");
            ImGui::Text("Block -> ID: 0");
            ImGui::Text("Block -> position: undefined");
            ImGui::Text("Block -> transparent: undefined");
            ImGui::Text("Block -> renderFacesInBetween: undefined");
            ImGui::Text("Block -> model name: undefined");
        }
        ImGui::Separator();
        ImGui::Text("Input -> Selected: %s", BlockDB::getBlockInfo(selectedBlockType)->name.c_str());
        ImGui::Text("Input -> Selected ID: %d", selectedBlockType);
        
        ImGui::PopStyleColor(2);
        ImGui::End();
    }

    // ---------------- Inventory window ----------------
    if (inventoryOpen) {
        ImGuiIO& io = ImGui::GetIO();

        const float slotSize = 64.0f;
        const float slotPad = 6.0f;
        const float slotGap = 4.0f;
        const uint8_t itemsPerRow = 6;
        const float gridWidth = itemsPerRow * (slotSize + slotPad * 2 + slotGap) - slotGap + 16.0f;
        const float winWidth = gridWidth + 20.0f; // window padding
        const float winHeight = io.DisplaySize.y * 0.65f;

        ImVec2 invPos(io.DisplaySize.x * 0.5f - winWidth * 0.5f, io.DisplaySize.y * 0.5f - winHeight * 0.5f);
        ImGui::SetNextWindowPos(invPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(winWidth, winHeight), ImGuiCond_Always);

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.10f, 0.12f, 0.96f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.45f, 0.45f, 0.55f, 0.70f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.5f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 10.0f));

        ImGui::Begin("Inventory",
                nullptr,
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoScrollbar);

        ImGui::PushStyleColor(ImGuiCol_Tab, ImVec4(0.14f, 0.14f, 0.18f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_TabHovered, ImVec4(0.30f, 0.40f, 0.55f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_TabActive, ImVec4(0.22f, 0.30f, 0.48f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_TabSelectedOverline,ImVec4(0.50f, 0.65f, 1.00f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_TabRounding, 4.0f);

        if (ImGui::BeginTabBar("InventoryTabs")) {
            for (const auto& tabName : tabOrder) {
                auto& indices = tabMap[tabName];
                if (ImGui::BeginTabItem(tabName.c_str())) {
                    ImGui::Spacing();
                    ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, ImVec4(0,0,0,0));
                    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, ImVec4(0,0,0,0));
                    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered, ImVec4(0,0,0,0));
                    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabActive, ImVec4(0,0,0,0));
                    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 0.0f);
                    ImGui::BeginChild("BlockGrid", ImVec2(0, 0), false, 0);
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(slotPad, slotPad));
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(slotGap, slotGap));

                    // Center the grid
                    {
                        const float totalSlotW = slotSize + slotPad * 2.0f;
                        const float rowW = itemsPerRow * totalSlotW + (itemsPerRow - 1) * slotGap;
                        const float availW = ImGui::GetContentRegionAvail().x;
                        const float offsetX = std::max(0.0f, (availW - rowW) * 0.5f);
                        ImGui::Indent(offsetX);
                    }

                    for (size_t n = 0; n < indices.size(); n++) {
                        size_t i = indices[n];

                        GLuint previewTex = BlockPreviewRenderer::getPreviewTexture(blockIds[i]);
                        ImTextureID texId = previewTex ? (ImTextureID)(intptr_t)previewTex : texAtlas;
                        ImVec2 uv0, uv1;

                        if (previewTex) {
                            uv0 = ImVec2(0.0f, 1.0f);
                            uv1 = ImVec2(1.0f, 0.0f);
                        } else {
                            // Fallback to atlas tile
                            const auto* blockInfo = BlockDB::getBlockInfo(blockIds[i]);
                            int tileX = static_cast<int>(blockInfo->textureCoords[0].x);
                            int tileY = static_cast<int>(blockInfo->textureCoords[0].y);
                            uv0 = ImVec2((tileX * 16) / 256.0f, ((tileY + 1) * 16) / 256.0f);
                            uv1 = ImVec2(((tileX + 1) * 16) / 256.0f, (tileY * 16) / 256.0f);
                        }

                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.16f, 0.16f, 0.20f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,ImVec4(0.28f, 0.38f, 0.55f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.35f, 0.50f, 0.75f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.35f, 0.35f, 0.45f, 0.80f));

                        if (ImGui::ImageButton(blockItems[i], texId, ImVec2(slotSize, slotSize), uv0, uv1)) {
                            setHotbarBlock(selectedHotbarIndex, blockIds[i]);
                            setSelectedBlockType(blockIds[i]);
                        }

                        ImGui::PopStyleColor(4);

                        if (ImGui::IsItemHovered()) {
                            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 6.0f));
                            ImGui::BeginTooltip();
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                            ImGui::TextUnformatted(blockItems[i]);
                            ImGui::PopStyleColor();
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.65f, 0.65f, 0.70f, 1.0f));
                            ImGui::Text("ID: %d", blockIds[i]);
                            ImGui::PopStyleColor();
                            ImGui::EndTooltip();
                            ImGui::PopStyleVar();
                        }

                        if ((n % itemsPerRow) != (itemsPerRow - 1) && n + 1 < indices.size())
                            ImGui::SameLine();
                    }
                    ImGui::PopStyleVar(4);

                    // Undo the centering indent
                    {
                        const float totalSlotW = slotSize + slotPad * 2.0f;
                        const float rowW = itemsPerRow * totalSlotW + (itemsPerRow - 1) * slotGap;
                        const float availW = ImGui::GetContentRegionAvail().x;
                        const float offsetX = std::max(0.0f, (availW - rowW) * 0.5f);
                        ImGui::Unindent(offsetX);
                    }

                    ImGui::EndChild();
                    ImGui::PopStyleVar(1); 
                    ImGui::PopStyleColor(4);
                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();
        }

        ImGui::PopStyleVar(1);
        ImGui::PopStyleColor(4);
        ImGui::End();
        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(2);
    }

    // ---------------- Hotbar UI ----------------
    if (hotbarOpen && !pauseMenuOpen) {
        ImGuiIO& io = ImGui::GetIO();
        const float slotSize = 54.0f;
        const float slotPad = 6.0f;
        const float slotGap = 4.0f;
        const int numSlots = (int)hotbarBlocks.size();
        const float winPadX = 10.0f;
        const float winPadY = 8.0f;
        const float totalSlotW = slotSize + slotPad * 2.0f;
        const float winWidth = numSlots * totalSlotW + (numSlots - 1) * slotGap + winPadX * 2.0f;
        const float winHeight = totalSlotW + winPadY * 2.0f;
        const float bottomGap = 8.0f;

        ImVec2 hotbarPos(io.DisplaySize.x * 0.5f - winWidth * 0.5f, io.DisplaySize.y - winHeight - bottomGap);

        ImGui::SetNextWindowPos(hotbarPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(winWidth, winHeight), ImGuiCond_Always);

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.08f, 0.10f, 0.82f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.45f, 0.45f, 0.55f, 0.60f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.5f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(winPadX, winPadY));

        ImGui::Begin("Hotbar",
            nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoScrollbar);

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(slotPad, slotPad));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(slotGap, slotGap));

        ImDrawList* drawList = ImGui::GetWindowDrawList();

        for (size_t i = 0; i < hotbarBlocks.size(); i++) {
            uint8_t blockId = hotbarBlocks[i];
            const auto* blockInfo = BlockDB::getBlockInfo(blockId);
            if (!blockInfo) continue;

            GLuint previewTex = BlockPreviewRenderer::getPreviewTexture(blockId);
            ImTextureID texId = previewTex ? (ImTextureID)(intptr_t)previewTex : texAtlas;
            ImVec2 uv0, uv1;

            if (previewTex) {
                uv0 = ImVec2(0.0f, 1.0f);
                uv1 = ImVec2(1.0f, 0.0f);
            } else {
                int tileX = static_cast<int>(blockInfo->textureCoords[0].x);
                int tileY = static_cast<int>(blockInfo->textureCoords[0].y);
                uv0 = ImVec2((tileX * 16) / 256.0f, ((tileY + 1) * 16) / 256.0f);
                uv1 = ImVec2(((tileX + 1) * 16) / 256.0f, (tileY * 16) / 256.0f);
            }

            bool isSelected = (i == selectedHotbarIndex);
            if (isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.22f, 0.28f, 0.40f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f, 0.28f, 0.40f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.22f, 0.28f, 0.40f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.55f, 0.75f, 1.00f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.14f, 0.18f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.14f, 0.14f, 0.18f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.14f, 0.14f, 0.18f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.32f, 0.32f, 0.40f, 0.85f));
            }

            std::string buttonLabel = "##hotbar_" + std::to_string(i);
            ImGui::ImageButton(buttonLabel.c_str(), texId, ImVec2(slotSize, slotSize), uv0, uv1);
            ImGui::PopStyleColor(4);

            // Draw slot number
            ImVec2 rMin = ImGui::GetItemRectMin();
            char numBuf[4];
            snprintf(numBuf, sizeof(numBuf), "%d", (int)(i + 1) % 10);
            ImVec2 numPos(rMin.x + 4.0f, rMin.y + 2.0f);
            drawList->AddText(ImVec2(numPos.x + 1.0f, numPos.y + 1.0f), IM_COL32(0, 0, 0, 180), numBuf);
            drawList->AddText(numPos, IM_COL32(200, 200, 200, 160), numBuf);

            if (isSelected) {
                ImVec2 rMin = ImGui::GetItemRectMin();
                ImVec2 rMax = ImGui::GetItemRectMax();
                drawList->AddRect(rMin, rMax, IM_COL32(140, 190, 255, 220), 5.0f, 0, 2.0f);
            }

            if (i < hotbarBlocks.size() - 1)
                ImGui::SameLine(0.0f, slotGap);
        }

        ImGui::PopStyleVar(4);
        ImGui::End();
        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(2);
    }

    // ---------------- Console window ----------------
    if (consoleOpen) {
        if (consoleLog.empty()) {
            consoleLog.push_back("Type 'help' for a list of commands.");
        }
        ImGuiIO& io = ImGui::GetIO();
        float consoleWidth = io.DisplaySize.x * 0.5f;
        float consoleHeight = io.DisplaySize.y * 0.45f;
        ImVec2 consolePos(7.0f, io.DisplaySize.y - consoleHeight - 7.0f);

        ImGui::SetNextWindowPos(consolePos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(consoleWidth, consoleHeight), ImGuiCond_Always);

        ImGui::Begin("ConsoleWindow",
                    nullptr,
                    ImGuiWindowFlags_NoTitleBar |
                    ImGuiWindowFlags_NoResize |
                    ImGuiWindowFlags_NoMove |
                    ImGuiWindowFlags_NoCollapse);

        ImGui::BeginChild("consoleLog", ImVec2(0, consoleHeight - 60), false, ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 0.0f));
        for (const auto& line : consoleLog) {
            ImGui::TextWrapped("%s", line.c_str());
        }
        ImGui::PopStyleVar();
        if (scrollToBottom) {
            ImGui::SetScrollHereY(1.0f);
            scrollToBottom = false;
        }
        ImGui::EndChild();

        ImGui::Separator();
        ImGui::PushItemWidth(-1);

        if (!prevConsoleOpen && consoleOpen) {
            consoleFocusDelayFrames = 5; // delay focus by 5 frames to avoid typing the toggle key
        }

        if (consoleFocusDelayFrames > 0) {
            consoleFocusDelayFrames--;
            if (consoleFocusDelayFrames == 0) {
                ImGui::SetKeyboardFocusHere();
            }
        }

        bool enterPressed = ImGui::InputText("##ConsoleInput", inputBuffer, IM_ARRAYSIZE(inputBuffer), ImGuiInputTextFlags_EnterReturnsTrue);

        if (enterPressed) {
            std::string input(inputBuffer);

            if (!input.empty()) {
                if (input.rfind("say ", 0) == 0) {
                    std::string msg = input.substr(4);
                    consoleLog.push_back(msg);
                } else if (input == "clear") {
                    consoleLog.clear();
                } else if (input == "help") {
                    consoleLog.push_back("Commands:");
                    consoleLog.push_back("  say <message> - Print a message");
                    consoleLog.push_back("  clear - Clear the console window");
                    consoleLog.push_back("  help - Show this help page");
                    consoleLog.push_back("  tp <x> <y> <z> - Teleport to coordinates");
                    consoleLog.push_back("  edgelands - Teleport to the edge of the world");
                } else if (input.rfind("tp", 0) == 0) {
                    std::istringstream ss(input);
                    std::string cmd, coordx, coordy, coordz;
                    ss >> cmd >> coordx >> coordy >> coordz;
                    if (coordx.empty() || coordy.empty() || coordz.empty()) {
                        consoleLog.push_back("Invalid usage. Use: tp <x> <y> <z> (~ for current position)");
                    } else {
                        glm::dvec3 current = camera.getPositionDouble();
                        float eyeHeight = camera.getEyeHeight();
                        double currentFeetY = current.y - eyeHeight;
                        auto parseCoord = [](const std::string& token, double currentVal, bool& ok) -> double {
                            ok = true;
                            if (token[0] == '~') {
                                if (token.size() == 1) return currentVal;
                                try {
                                    return currentVal + std::stof(token.substr(1));
                                } catch (...) {
                                    ok = false;
                                    return 0.0f;
                                }
                            }
                            try {
                                return std::stof(token);
                            } catch (...) {
                                ok = false;
                                return 0.0f;
                            }
                        };
                        bool okx = true, oky = true, okz = true;
                        double x = parseCoord(coordx, current.x, okx);
                        double y = parseCoord(coordy, currentFeetY, oky);
                        double z = parseCoord(coordz, current.z, okz);
                        if (!okx || !oky || !okz) {
                            consoleLog.push_back("Invalid coordinates. Example: tp ~ ~10 ~-5");
                        } else {
                            camera.setPosition(glm::dvec3(x+0.5, y + eyeHeight, z+0.5));
                            char buf[128];
                            snprintf(buf, sizeof(buf), "Teleported to %.2f %.2f %.2f", x, y, z);
                            consoleLog.push_back(buf);
                        }
                    }
                } /*else if (input == "farlands"){
                    camera.setPosition(glm::vec3(4294960.0f, 100.0f, 0));
                }*/ else if (input == "edgelands"){
                    camera.setPosition(glm::dvec3(2147483635.0, 100.0, 0));
                    consoleLog.push_back("Do not step on blocks right at the edge (game will crash)");
                } else {
                    consoleLog.push_back("Unknown command. Type 'help' for a list of commands.");
                }
                inputBuffer[0] = '\0';
                scrollToBottom = true;
            }
            ImGui::SetKeyboardFocusHere(-1);
        }

        ImGui::PopItemWidth();
        ImGui::End();
    }
    prevConsoleOpen = consoleOpen;

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}