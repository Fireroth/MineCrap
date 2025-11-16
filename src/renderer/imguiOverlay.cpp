#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <sstream>
#include "../world/world.hpp"
#include "../core/camera.hpp"
#include "../world/blockDB.hpp"
#include "imguiOverlay.hpp"
#include "../world/block_interaction.hpp"
#include "../core/input.hpp"
#include "../core/options.hpp"

const float ImGuiOverlay::fpsRefreshInterval = 0.5f; // 500ms

std::vector<const char*> ImGuiOverlay::blockItems;
std::vector<uint8_t> ImGuiOverlay::blockIds;
ImTextureID ImGuiOverlay::texAtlas;

static std::vector<std::string> consoleLog;
static char inputBuffer[256] = "";
static bool scrollToBottom = false;

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

    ImFont* Font = io.Fonts->AddFontFromFileTTF("./Font.ttf", 25.0f);

    return true;
}

void ImGuiOverlay::render(float deltaTime, Camera& camera, World* world) {
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
        
        ImVec2 windowSize = ImGui::GetWindowSize();

        ImVec2 buttonSize(200, 50);
        float spacing = 20.0f;

        float startY = (windowSize.y - (buttonSize.y * 4 + spacing * 2)) * 0.5f;
        float centerX = (windowSize.x - buttonSize.x) * 0.5f;

        switch (pauseScreenPage) {
            case PauseMenuPage::Main: {
                ImGui::SetCursorPos(ImVec2(centerX, startY));
                if (ImGui::Button("Resume", buttonSize)) {
                    pauseMenuOpen = false;
                    cursorCaptured = true;
                    glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                }

                ImGui::SetCursorPos(ImVec2(centerX, startY + buttonSize.y + spacing));
                if (ImGui::Button("Settings", buttonSize)) {
                    pauseScreenPage = PauseMenuPage::Settings;
                }

                ImGui::SetCursorPos(ImVec2(centerX, startY + 2*(buttonSize.y + spacing)));
                if (ImGui::Button("Controls", buttonSize)) {
                    pauseScreenPage = PauseMenuPage::Controls;
                }

                ImGui::SetCursorPos(ImVec2(centerX, startY + 3*(buttonSize.y + spacing)));
                if (ImGui::Button("Quit", buttonSize)) {
                    glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
                }
            } break;

            case PauseMenuPage::Settings: {
                float totalHeight = ImGui::GetTextLineHeightWithSpacing() + spacing + 2 * buttonSize.y + spacing;
                float startY = (windowSize.y - totalHeight) * 0.5f;
                float centerX = (windowSize.x - buttonSize.x) * 0.5f;

                ImGui::SetCursorPos(ImVec2(centerX, startY));
                ImGui::Text("Settings");

                ImGui::SetCursorPos(ImVec2(centerX, startY + ImGui::GetTextLineHeightWithSpacing() + spacing));
                if (ImGui::Button("Video", buttonSize))
                    pauseScreenPage = PauseMenuPage::Video;

                ImGui::SetCursorPos(ImVec2(centerX, startY + ImGui::GetTextLineHeightWithSpacing() + spacing + buttonSize.y + spacing));
                if (ImGui::Button("Back", buttonSize))
                    pauseScreenPage = PauseMenuPage::Main;
            } break;

            case PauseMenuPage::Video: {
                float totalHeight = ImGui::GetTextLineHeightWithSpacing() + spacing + 30 + spacing + buttonSize.y;
                float startY = (windowSize.y - totalHeight) * 0.5f;
                float centerX = (windowSize.x - buttonSize.x) * 0.5f;

                ImGui::SetCursorPos(ImVec2(centerX, startY));
                ImGui::Text("Video Settings");

                int renderDistance = getOptionInt("render_distance", 7);
                float sliderWidth = 250.0f;

                const char* label = "Render Distance";
                float labelWidth = ImGui::CalcTextSize(label).x;
                float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
                float totalWidth = labelWidth + spacing + sliderWidth;
                float startX = (windowSize.x - totalWidth) * 0.5f;

                float sliderY = startY + ImGui::GetTextLineHeightWithSpacing() + spacing;
                ImGui::SetCursorPos(ImVec2(startX, sliderY));

                ImGui::TextUnformatted(label);
                ImGui::SameLine();

                ImGui::PushItemWidth(sliderWidth);
                if (ImGui::SliderInt("##RenderDistance", &renderDistance, 1, 32)) {
                    saveOption("render_distance", renderDistance, "options.txt");
                    world->updateChunksAroundPlayer(camera.getPosition(), renderDistance, true);
                }
                ImGui::PopItemWidth();

                float fov = getOptionFloat("fov", 70.0f);
                const char* fovLabel = "Field of View";
                float fovLabelWidth = ImGui::CalcTextSize(fovLabel).x;
                float fovTotalWidth = fovLabelWidth + spacing + sliderWidth;
                float fovStartX = (windowSize.x - fovTotalWidth) * 0.5f;
                float fovSliderY = sliderY + 50;
                ImGui::SetCursorPos(ImVec2(fovStartX, fovSliderY));
                ImGui::TextUnformatted(fovLabel);
                ImGui::SameLine();
                ImGui::PushItemWidth(sliderWidth);
                if (ImGui::SliderFloat("##FOV", &fov, 10.0f, 110.0f, "%.1f")) {
                    saveOption("fov", static_cast<int>(fov), "options.txt");
                }
                ImGui::PopItemWidth();

                float backButtonY = fovSliderY + 50;
                ImGui::SetCursorPos(ImVec2(centerX, backButtonY));
                if (ImGui::Button("Back", buttonSize))
                    pauseScreenPage = PauseMenuPage::Settings;
            } break;

            case PauseMenuPage::Controls: {
                ImGuiIO& io = ImGui::GetIO();
                float infoWidth = 400.0f;
                float infoHeight = io.DisplaySize.y * 0.6f;
                float startY = (windowSize.y - infoHeight) * 0.5f;
                float centerX = (windowSize.x - infoWidth) * 0.5f;

                ImGui::SetCursorPos(ImVec2(centerX, startY));
                ImGui::BeginChild("ControlsInfo", ImVec2(infoWidth, infoHeight), true);
                ImGui::Text("Controls");
                ImGui::Separator();
                ImGui::Text("W/A/S/D: Move");
                ImGui::Text("Space: Up/Jump");
                ImGui::Text("Shift: Down (in Fly mode)");
                ImGui::Text("F: Toggle Fly Mode");
                ImGui::Text("G: Toggle Wireframe Mode");
                ImGui::Text("Ctrl: Sprint");
                ImGui::Text("E: Open Inventory");
                ImGui::Text("Esc: Pause Menu");
                ImGui::Text("Left Mouse: Break Block");
                ImGui::Text("Right Mouse: Place Block");
                ImGui::Text("Middle Mouse: Pick Block");
                ImGui::Text("1-9: Select Block");
                ImGui::Text("F1: Toggle Hotbar and Crosshair");
                ImGui::Text("F3: Debug Info");
                ImGui::Text("T: Console");
                ImGui::EndChild();

                ImGui::SetCursorPos(ImVec2(centerX, startY + infoHeight + spacing));
                if (ImGui::Button("Back", buttonSize))
                    pauseScreenPage = PauseMenuPage::Main;
            } break;
        }

        ImGui::End();

        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor();
    }

    prevPauseMenuOpen = pauseMenuOpen;

    // ---------------- Debug window ----------------
    if (debugOpen) {
        ImGui::SetNextWindowSize(ImVec2(450, 0)); // Width, Height
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        
        glm::vec3 pos = camera.getPosition();
        glm::vec3 front = camera.getFront();
        glm::vec3 up = camera.getUp();
        BlockInfo blockInfo = getLookedAtBlockInfo(world, camera);
        float camYaw = camera.getYaw();
        float camPitch = camera.getPitch();
        bool grounded = camera.isGrounded();

        uint8_t selectedBlockType = getSelectedBlockType();

        float eyeHeight = camera.getEyeHeight();
        glm::vec3 feetPos = glm::vec3(pos.x, pos.y - eyeHeight, pos.z);
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
        ImGui::Text("Pos: %.2f / %.2f / %.2f", feetPos.x, feetPos.y, feetPos.z);
        ImGui::Text("Delta Time: %.2f ms", deltaTime*1000);
        ImGui::Text("Chunk: %d, %d", chunkX, chunkZ);
        ImGui::Separator();
        ImGui::Text("Camera -> Yaw: %.2f", camYaw);
        ImGui::Text("Camera -> Pitch: %.2f", camPitch);
        ImGui::Text("Camera -> Front: %.2f, %.2f, %.2f", front.x, front.y, front.z);
        ImGui::Text("Camera -> Up: %.2f, %.2f, %.2f", up.x, up.y, up.z);
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
        float winWidth = 500.0f;
        float winHeight = io.DisplaySize.y * 0.6f;
        ImVec2 invPos(
            io.DisplaySize.x * 0.5f - winWidth * 0.5f,
            io.DisplaySize.y * 0.5f - winHeight * 0.5f
        );
        ImGui::SetNextWindowPos(invPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(winWidth, winHeight), ImGuiCond_Always);
        ImGui::Begin("##Inventory",
                nullptr,
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoCollapse);

        if (ImGui::BeginTabBar("InventoryTabs")) {
            for (const auto& tabName : tabOrder) {
                auto& indices = tabMap[tabName];
                if (ImGui::BeginTabItem(tabName.c_str())) {

                    ImGui::BeginChild("BlockGrid", ImVec2(0, 0), false);

                    for (size_t n = 0; n < indices.size(); n++) {
                        size_t i = indices[n];
                        const auto* blockInfo = BlockDB::getBlockInfo(blockIds[i]);
                        int tileX = static_cast<int>(blockInfo->textureCoords[0].x);
                        int tileY = static_cast<int>(blockInfo->textureCoords[0].y);

                        ImVec2 uv0 = ImVec2(
                            (tileX * 16) / 256.0f,
                            ((tileY + 1) * 16) / 256.0f
                        );
                        ImVec2 uv1 = ImVec2(
                            ((tileX + 1) * 16) / 256.0f,
                            (tileY * 16) / 256.0f
                        );

                        if (ImGui::ImageButton(blockItems[i], texAtlas, ImVec2(64, 64), uv0, uv1)) {
                            setHotbarBlock(selectedHotbarIndex, blockIds[i]);
                            setSelectedBlockType(blockIds[i]);
                        }

                        if (ImGui::IsItemHovered())
                            ImGui::SetTooltip("%s", blockItems[i]);

                        static uint8_t itemsPerRow = 6;
                        if ((n % itemsPerRow) != (itemsPerRow - 1) && n + 1 < indices.size())
                            ImGui::SameLine();
                    }
                    ImGui::EndChild();
                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();
        }

        ImGui::End();
    }

    // ---------------- Hotbar UI ----------------
    if (hotbarOpen && !pauseMenuOpen) {
        ImGuiIO& io = ImGui::GetIO();
        const float slotSize = 54.0f;
        const float padding = 3.0f;
        const float winWidth = 650.0f;
        const float winHeight = 85.0f;

        ImVec2 hotbarPos(
            io.DisplaySize.x * 0.5f - winWidth * 0.5f,
            io.DisplaySize.y - winHeight - 0.0f
        );

        ImGui::SetNextWindowPos(hotbarPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(winWidth, winHeight), ImGuiCond_Always);

        ImGui::Begin("##Hotbar",
            nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoScrollbar);

        for (size_t i = 0; i < hotbarBlocks.size(); i++) {
            uint8_t blockId = hotbarBlocks[i];
            const auto* blockInfo = BlockDB::getBlockInfo(blockId);
            if (!blockInfo) continue;

            int tileX = static_cast<int>(blockInfo->textureCoords[0].x);
            int tileY = static_cast<int>(blockInfo->textureCoords[0].y);

            ImVec2 uv0 = ImVec2(
                (tileX * 16) / 256.0f,
                ((tileY + 1) * 16) / 256.0f
            );
            ImVec2 uv1 = ImVec2(
                ((tileX + 1) * 16) / 256.0f,
                (tileY * 16) / 256.0f
            );

            bool isSelected = (i == selectedHotbarIndex);
            if (isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.5f, 1.0f, 0.9f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.5f, 1.0f, 0.9f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.5f, 1.0f, 0.9f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 0.9f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 0.9f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.2f, 0.2f, 0.9f));
            }

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(7.0f, 7.0f));

            std::string buttonLabel = "##" + std::to_string(i);
            ImGui::ImageButton(buttonLabel.c_str(), texAtlas, ImVec2(slotSize, slotSize), uv0, uv1);
            ImGui::PopStyleVar();

            ImGui::PopStyleColor(3);

            if (i < hotbarBlocks.size() - 1)
                ImGui::SameLine(0.0f, padding);
        }
        ImGui::End();
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

        bool enterPressed = ImGui::InputText("##ConsoleInput",
                                            inputBuffer,
                                            IM_ARRAYSIZE(inputBuffer),
                                            ImGuiInputTextFlags_EnterReturnsTrue);

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
                    consoleLog.push_back("  farlands - Teleport to Far Lands");
                } else if (input.rfind("tp", 0) == 0) {
                    std::istringstream ss(input);
                    std::string cmd, coordx, coordy, coordz;
                    ss >> cmd >> coordx >> coordy >> coordz;
                    if (coordx.empty() || coordy.empty() || coordz.empty()) {
                        consoleLog.push_back("Invalid usage. Use: tp <x> <y> <z> (~ for current position)");
                    } else {
                        glm::vec3 current = camera.getPosition();
                        float eyeHeight = camera.getEyeHeight();
                        float currentFeetY = current.y - eyeHeight;
                        auto parseCoord = [](const std::string& token, float currentVal, bool& ok) -> float {
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
                        float x = parseCoord(coordx, current.x, okx);
                        float y = parseCoord(coordy, currentFeetY, oky);
                        float z = parseCoord(coordz, current.z, okz);
                        if (!okx || !oky || !okz) {
                            consoleLog.push_back("Invalid coordinates. Example: tp ~ ~10 ~-5");
                        } else {
                            camera.setPosition(glm::vec3(x, y + eyeHeight, z));
                            char buf[128];
                            snprintf(buf, sizeof(buf), "Teleported to %.2f %.2f %.2f", x, y, z);
                            consoleLog.push_back(buf);
                        }
                    }
                } else if (input == "farlands"){
                    camera.setPosition(glm::vec3(4294960.0f, 100.0f, 0));
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
