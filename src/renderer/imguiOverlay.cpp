#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <string>
#include "../world/blockDB.hpp"
#include "ImGuiOverlay.hpp"
#include "../world/block_interaction.hpp"
#include "../core/input.hpp"

const float ImGuiOverlay::fpsRefreshInterval = 0.5f; // 500ms

std::vector<const char*> ImGuiOverlay::blockItems;
std::vector<uint8_t> ImGuiOverlay::blockIds;
ImTextureID ImGuiOverlay::texAtlas;

ImGuiOverlay::ImGuiOverlay()
    : fpsTimer(0.0f), frameCount(0), fpsDisplay(0.0f) {}

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

    texAtlas = (ImTextureID)(intptr_t)textureAtlas;

    return true;
}

void ImGuiOverlay::render(float deltaTime, const Camera& camera, World* world) {
    frameCount++;
    fpsTimer += deltaTime;

    if (fpsTimer >= fpsRefreshInterval) {
        fpsDisplay = frameCount / fpsTimer;
        frameCount = 0;
        fpsTimer = 0.0f;
    }
    
    if (!(inventoryOpen || debugOpen)) return;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // ---------------- Debug window ----------------
    if (debugOpen) {

        ImGui::SetNextWindowSize(ImVec2(320, 0)); // Width, Height
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        
        glm::vec3 pos = camera.getPosition();
        glm::vec3 front = camera.getFront();
        glm::vec3 up = camera.getUp();
        BlockInfo blockInfo = getLookedAtBlockInfo(world, camera);
        float camYaw = camera.getYaw();
        float camPitch = camera.getPitch();

        // Calculate chunk coordinates
        int chunkX = static_cast<int>(std::floor(pos.x / 16.0f));
        int chunkZ = static_cast<int>(std::floor(pos.z / 16.0f));

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 0.5f));
        ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(1.0f, 1.0f, 1.0f, 0.7f));

        ImGui::Begin("Debug",
                    nullptr,
                    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        ImGui::Text("FPS: %.1f", fpsDisplay);
        ImGui::Text("Pos: %.2f / %.2f / %.2f", pos.x, pos.y, pos.z);
        ImGui::Text("Delta Time: %.2f ms", deltaTime*1000);
        ImGui::Text("Chunk: %d, %d", chunkX, chunkZ);
        ImGui::Separator();
        ImGui::Text("Camera -> Yaw: %.2f", camYaw);
        ImGui::Text("Camera -> Pitch: %.2f", camPitch);
        ImGui::Text("Camera -> Front: %.2f, %.2f, %.2f", front.x, front.y, front.z);
        ImGui::Text("Camera -> Up: %.2f, %.2f, %.2f", up.x, up.y, up.z);
        ImGui::Separator();
        if (blockInfo.valid) {
            const auto* info = BlockDB::getBlockInfo(blockInfo.type);
            ImGui::Text("Block -> name: %s", info->name.c_str());
            ImGui::Text("Block -> ID: %d", blockInfo.type);
            ImGui::Text("Block -> position: %d / %d / %d", blockInfo.worldPos.x, blockInfo.worldPos.y, blockInfo.worldPos.z);
            ImGui::Text("Block -> transparent: %s", info->transparent ? "True" : "False");
            ImGui::Text("Block -> renderFacesInBetween: %s", info->renderFacesInBetween ? "True" : "False");
            ImGui::Text("Block -> model name: %s", info->modelName);
        } else {
            ImGui::Text("Block -> name: Air");
            ImGui::Text("Block -> ID: 0");
            ImGui::Text("Block -> position: undefined");
            ImGui::Text("Block -> transparent: undefined");
            ImGui::Text("Block -> renderFacesInBetween: undefined");
            ImGui::Text("Block -> model name: undefined");
        }
        ImGui::PopStyleColor(2);
        ImGui::End();
    }

    // ---------------- Inventory window ----------------
    if (inventoryOpen) {
        ImGuiIO& io = ImGui::GetIO();
        float winWidth = 425.0f;
        float winHeight = io.DisplaySize.y * 0.6f;
        ImVec2 invPos(
            io.DisplaySize.x * 0.5f - winWidth * 0.5f,
            io.DisplaySize.y * 0.5f - winHeight * 0.5f
        );
        ImGui::SetNextWindowPos(invPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(winWidth, winHeight), ImGuiCond_Always);
        ImGui::Begin("##Inventory",
                nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        uint8_t selectedBlockType = getSelectedBlockType();
        for (size_t i = 0; i < blockItems.size(); ++i) {
            bool isSelected = (selectedBlockType == blockIds[i]);
            if (isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 1.0f, 1.0f, 0.4f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 1.0f, 1.0f, 0.8f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 1.0f, 1.0f, 0.4f));
            }

            ImGui::BeginGroup();
            const auto* blockInfo = BlockDB::getBlockInfo(blockIds[i]);
            int tileX = static_cast<int>(blockInfo->textureCoords[0].x);
            int tileY = static_cast<int>(blockInfo->textureCoords[0].y);

            ImVec2 uv0 = ImVec2(
                (tileX * 16) / (float)256,
                ((tileY + 1) * 16) / (float)256
            );
            ImVec2 uv1 = ImVec2(
                ((tileX + 1) * 16) / (float)256,
                (tileY * 16) / (float)256
            );
            if (ImGui::ImageButton(blockItems[i], texAtlas, ImVec2(64, 64), uv0, uv1)) {
                setSelectedBlockType(blockIds[i]);
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(blockItems[i]);
            ImGui::EndGroup();

            if (isSelected) {
                ImGui::PopStyleColor(3);
            }
            static int itemsPerRow = 5;
            if ((i % itemsPerRow) != (itemsPerRow - 1) && i + 1 < blockItems.size()) {
                ImGui::SameLine();
            }
        }
        ImGui::End();
    }

    if (inventoryOpen || debugOpen) {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}
