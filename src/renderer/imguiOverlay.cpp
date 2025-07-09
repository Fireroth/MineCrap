#include <imgui.h>
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

ImGuiOverlay::ImGuiOverlay()
    : fpsTimer(0.0f), frameCount(0), fpsDisplay(0.0f) {}

ImGuiOverlay::~ImGuiOverlay() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

bool ImGuiOverlay::init(GLFWwindow* window) {
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

    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowSize(ImVec2(280, 150)); // Width, Height
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    
    glm::vec3 pos = camera.getPosition();
    glm::vec3 front = camera.getFront();
    BlockInfo blockInfo = getLookedAtBlockInfo(world, camera);

    // Calculate chunk coordinates
    int chunkX = static_cast<int>(std::floor(pos.x / 16.0f));
    int chunkZ = static_cast<int>(std::floor(pos.z / 16.0f));

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 0.5f));
    ImGui::Begin("Debug",
                nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    ImGui::Text("FPS: %.1f", fpsDisplay);
    ImGui::Text("Camera Pos: %.2f, %.2f, %.2f", pos.x, pos.y, pos.z);
    ImGui::Text("Camera Front: %.2f, %.2f, %.2f", front.x, front.y, front.z);
    ImGui::Text("Camera Yaw: %.2f", camera.getYaw());
    ImGui::Text("Camera Pitch: %.2f", camera.getPitch());
    ImGui::Text("Chunk: %d, %d", chunkX, chunkZ);

    if (blockInfo.valid) {
        const auto* info = BlockDB::getBlockInfo(blockInfo.type);
        ImGui::Text("Looking at: %s", info->name.c_str());
        ImGui::Text("Block position: [%d, %d, %d]", blockInfo.worldPos.x, blockInfo.worldPos.y, blockInfo.worldPos.z);
    } else {
        ImGui::Text("Looking at: nothing");
        ImGui::Text("Block position: undefined");
    }

    ImGui::PopStyleColor();
    ImGui::End();

    // ---------------- Inventory window ----------------
    if (inventoryOpen) {
        ImGuiIO& io = ImGui::GetIO();
        float winWidth = 465.0f;
        float winHeight = io.DisplaySize.y * 0.5f;
        ImVec2 invPos(
            io.DisplaySize.x * 0.5f - winWidth * 0.5f,
            io.DisplaySize.y * 0.5f - winHeight * 0.5f
        );
        ImGui::SetNextWindowPos(invPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(winWidth, winHeight), ImGuiCond_Always);
        ImGui::Begin("##Inventory",
                nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        int currentId = getSelectedBlockType()-1;

        ImVec2 buttonSize(140, 23);
        for (size_t i = 0; i < blockItems.size(); ++i) {
            bool isSelected = (getSelectedBlockType() == blockIds[i]);
            if (isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 1.0f, 1.0f, 0.4f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 1.0f, 1.0f, 0.8f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 1.0f, 1.0f, 0.4f));
            }
            if (ImGui::Button(blockItems[i], buttonSize)) {
                setSelectedBlockType(blockIds[i]);
            }
            if (isSelected) {
                ImGui::PopStyleColor(3);
            }
            // Place three buttons per line
            if (i % 3 != 2 && i + 1 < blockItems.size()) {
                ImGui::SameLine();
            }
        }
        ImGui::End();

    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
