#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <map>
#include <string>
#include "ImGuiOverlay.hpp"
#include "../world/block_interaction.hpp"
#include "../core/input.hpp"
#include <vector>

std::map<uint8_t, std::string> blockNames = {
    { 1, "Grass" },
    { 2, "Dirt" },
    { 3, "Stone" },
    { 4, "Sand" },
    { 5, "Log" },
    { 6, "Bedrock" },
    { 7, "Gravel"},
    { 8, "Bricks"},
    { 9, "Water"},
    { 10, "Lava"},
    { 11, "Leaves"},
    { 12, "Cactus"}
};

const float ImGuiOverlay::fpsRefreshInterval = 0.5f; // 500ms

ImGuiOverlay::ImGuiOverlay()
    : fpsTimer(0.0f), frameCount(0), fpsDisplay(0.0f) {}

ImGuiOverlay::~ImGuiOverlay() {
    shutdown();
}

bool ImGuiOverlay::init(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

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

    ImGui::SetNextWindowSize(ImVec2(285, 190)); // Width: 285, Height: 190
    
    glm::vec3 pos = camera.getPosition();
    glm::vec3 front = camera.getFront();
    BlockInfo blockInfo = getLookedAtBlockInfo(world, camera);

    // Calculate chunk coordinates
    int chunkX = static_cast<int>(std::floor(pos.x / 16.0f));
    int chunkZ = static_cast<int>(std::floor(pos.z / 16.0f));

    ImGui::Begin("Debug");
    ImGui::Text("FPS: %.1f", fpsDisplay);
    ImGui::Text("Camera Pos: %.2f, %.2f, %.2f", pos.x, pos.y, pos.z);
    ImGui::Text("Camera Front: %.2f, %.2f, %.2f", front.x, front.y, front.z);
    ImGui::Text("Camera Yaw: %.2f", camera.getYaw());
    ImGui::Text("Camera Pitch: %.2f", camera.getPitch());
    ImGui::Text("Chunk: %d, %d", chunkX, chunkZ);

    if (blockInfo.valid) {
        ImGui::Text("Looking at: %s", blockNames[blockInfo.type].c_str());
        ImGui::Text("Block position: [%d, %d, %d]", blockInfo.worldPos.x, blockInfo.worldPos.y, blockInfo.worldPos.z);
    } else {
        ImGui::Text("Looking at: nothing");
        ImGui::Text("Block position: undefined");
    }

    // Dropdown for block selection
    static std::vector<const char*> blockItems;
    static std::vector<uint8_t> blockIds;
    if (blockItems.empty()) {
        for (const auto& [id, name] : blockNames) {
            blockItems.push_back(name.c_str());
            blockIds.push_back(id);
        }
    }
    int currentIdx = 0;
    uint8_t selectedBlock = getSelectedBlockType();
    for (size_t i = 0; i < blockIds.size(); ++i) {
        if (blockIds[i] == selectedBlock) {
            currentIdx = static_cast<int>(i);
            break;
        }
    }
    ImGui::Text("Selected block:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(150);
    if (ImGui::Combo("##SelectedBlockCombo", &currentIdx, blockItems.data(), static_cast<int>(blockItems.size()))) {
        setSelectedBlockType(blockIds[currentIdx]);
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiOverlay::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
