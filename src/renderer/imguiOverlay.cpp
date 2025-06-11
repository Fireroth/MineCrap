#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <map>
#include <string>
#include "ImGuiOverlay.hpp"
#include "../world/block_interaction.hpp"
#include "../core/input.hpp"

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

    ImGui::SetNextWindowSize(ImVec2(300, 205)); // Width: 300, Height: 205
    
    glm::vec3 pos = camera.getPosition();
    glm::vec3 front = camera.getFront();
    BlockInfo blockInfo = getLookedAtBlockInfo(world, camera);

    // Calculate chunk coordinates
    int chunkX = static_cast<int>(std::floor(pos.x / 16.0f));
    int chunkZ = static_cast<int>(std::floor(pos.z / 16.0f));

    ImGui::Begin("Debug");
    ImGui::Text("FPS: %.1f", fpsDisplay);
    ImGui::Text("fpsTimer: %.1f", fpsTimer);
    ImGui::Text("Cam Pos: %.2f, %.2f, %.2f", pos.x, pos.y, pos.z);
    ImGui::Text("Cam Front: %.2f, %.2f, %.2f", front.x, front.y, front.z);
    ImGui::Text("Cam Yaw: %.2f", camera.getYaw());
    ImGui::Text("Cam Pitch: %.2f", camera.getPitch());
    ImGui::Text("Chunk: %d, %d", chunkX, chunkZ);

    if (blockInfo.valid) {
        ImGui::Text("Looking at block: %s", blockNames[blockInfo.type].c_str());
        ImGui::Text("Block position: [%d, %d, %d]", blockInfo.worldPos.x, blockInfo.worldPos.y, blockInfo.worldPos.z);
    } else {
        ImGui::Text("Looking at: nothing");
        ImGui::Text("Block position: undefined");
    }

    ImGui::Text("Selected block: %s", blockNames[getSelectedBlockType()].c_str());

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiOverlay::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
