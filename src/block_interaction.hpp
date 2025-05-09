#pragma once

#include <glm/glm.hpp>
#include <string>

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
    std::string type;
};

RaycastResult raycast(World* world, const glm::vec3& origin, const glm::vec3& dir, float maxDistance);

void breakBlockOnClick(World* world, const Camera& camera);
void placeBlockOnClick(World* world, const Camera& camera);

// For ImGui
BlockInfo getLookedAtBlockInfo(World* world, const Camera& camera);