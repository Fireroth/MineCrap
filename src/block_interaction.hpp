#pragma once

#include <glm/glm.hpp>

class Camera;
class Chunk;
class World;

struct RaycastResult {
    bool hit = false;
    glm::ivec3 hitBlockPos;
    Chunk* hitChunk = nullptr;

    bool hasPlacePos = false;
    glm::ivec3 placeBlockPos;
    Chunk* placeChunk = nullptr;

    glm::ivec3 faceNormal = glm::ivec3(0);
};

struct BlockInfo {
    bool valid = false;
    glm::ivec3 worldPos;
    uint8_t type;
};

RaycastResult raycast(World* world, const glm::vec3& origin, const glm::vec3& dir, float maxDistance);

void placeBreakBlockOnClick(World* world, const Camera& camera, char action, uint8_t blockType);

// For ImGui
BlockInfo getLookedAtBlockInfo(World* world, const Camera& camera);