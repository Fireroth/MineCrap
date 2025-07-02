#pragma once

#include <glm/glm.hpp>
#include <string>

class Camera;
class Chunk;
class World;

struct AABB {
    glm::vec3 min;
    glm::vec3 max;
};

AABB getModelAABB(const std::string& modelName);

bool rayAABBIntersect(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                      const glm::vec3& boxMin, const glm::vec3& boxMax,
                      float& tmin, float maxDistance);

struct RaycastResult {
    bool hit = false;
    glm::ivec3 hitBlockPos;
    Chunk* hitChunk = nullptr;

    bool hasPlacePos = false;
    glm::ivec3 placeBlockPos;
    Chunk* placeChunk = nullptr;

    glm::ivec3 faceNormal = glm::ivec3(0);
};

glm::ivec3 getAABBHitNormal(const glm::vec3& hitPoint, const glm::vec3& boxMin, const glm::vec3& boxMax);
int worldToChunkCoord(int x, int chunkSize);
RaycastResult raycast(World* world, const glm::vec3& origin, const glm::vec3& dir, float maxDistance);
void placeBreakBlockOnClick(World* world, const Camera& camera, char action, uint8_t blockType);

struct BlockInfo {
    bool valid = false;
    glm::ivec3 worldPos;
    uint8_t type;
};

BlockInfo getLookedAtBlockInfo(World* world, const Camera& camera);
