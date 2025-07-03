#include <glad/glad.h>
#include "chunk.hpp"
#include "../core/camera.hpp"
#include "world.hpp"
#include "blockDB.hpp"

struct AABB {
    glm::vec3 min;
    glm::vec3 max;
};

struct RaycastResult {
    bool hit = false;
    glm::ivec3 hitBlockPos;
    Chunk* hitChunk = nullptr;

    bool hasPlacePos = false;
    glm::ivec3 placeBlockPos;
    Chunk* placeChunk = nullptr;

    glm::ivec3 faceNormal = glm::ivec3(0);
};


// Helper function to get AABB for a block type
AABB getModelAABB(uint8_t blockId) {
    const BlockDB::BlockInfo* info = BlockDB::getBlockInfo(blockId);
    if (info->liquid) {
        return {glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(1.0f, 0.85f, 1.0f)};
    } else if (info->modelName == "cactus") {
        return {glm::vec3(0.1f, 0.0f, 0.1f),
                glm::vec3(0.9f, 1.0f, 0.9f)};
    } else if (info->modelName == "cross") {
        return {glm::vec3(0.1f, 0.0f, 0.1f),
                glm::vec3(0.9f, 0.8f, 0.9f)};
    } else if (info->modelName == "slab") {
        return {glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(1.0f, 0.5f, 1.0f)};
    } else { // full block
        return {glm::vec3(0.0f),
                glm::vec3(1.0f)};
    }
}

// Ray-AABB intersection helper
bool rayAABBIntersect( const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& boxMin, const glm::vec3& boxMax, float& hitDist, float maxRayDist) {
    float nearestEntry = 0.0f;
    float farthestExit = maxRayDist;
    for (int axis = 0; axis < 3; ++axis) {
        float inverseDir = 1.0f / rayDir[axis];
        float entryDist = (boxMin[axis] - rayOrigin[axis]) * inverseDir;
        float exitDist  = (boxMax[axis] - rayOrigin[axis]) * inverseDir;
        if (entryDist > exitDist) std::swap(entryDist, exitDist);
        nearestEntry = std::max(nearestEntry, entryDist);
        farthestExit = std::min(farthestExit, exitDist);
        if (farthestExit < nearestEntry) return false;
    }

    hitDist = nearestEntry;
    return nearestEntry <= maxRayDist && farthestExit >= 0.0f;
}

// Helper function that returns the face normal of the AABB that was hit
glm::ivec3 getAABBHitNormal(const glm::vec3& hitPoint, const glm::vec3& boxMin, const glm::vec3& boxMax) {
    const float helper = 0.0001f;
    if (fabs(hitPoint.x - boxMin.x) < helper) return glm::ivec3(-1, 0, 0);
    if (fabs(hitPoint.x - boxMax.x) < helper) return glm::ivec3(1, 0, 0);
    if (fabs(hitPoint.y - boxMin.y) < helper) return glm::ivec3(0, -1, 0);
    if (fabs(hitPoint.y - boxMax.y) < helper) return glm::ivec3(0, 1, 0);
    if (fabs(hitPoint.z - boxMin.z) < helper) return glm::ivec3(0, 0, -1);
    if (fabs(hitPoint.z - boxMax.z) < helper) return glm::ivec3(0, 0, 1);
    return glm::ivec3(0, 0, 0); // fallback
}

// Helper function for correct chunk coordinate calculation
int worldToChunkCoord(int x, int chunkSize) {
    return (x >= 0) ? (x / chunkSize) : ((x - chunkSize + 1) / chunkSize);
}

RaycastResult raycast(World* world, const glm::vec3& origin, const glm::vec3& dir, float maxDist) {
    RaycastResult result;

    glm::vec3 rayPos = origin;
    glm::ivec3 blockPos = glm::floor(rayPos);

    glm::vec3 deltaDist = glm::abs(glm::vec3(1.0f) / dir);
    glm::ivec3 step = glm::sign(dir);

    glm::vec3 sideDist;
    for (int i = 0; i < 3; ++i) {
        float offset = (step[i] > 0 ? (blockPos[i] + 1.0f - rayPos[i]) : (rayPos[i] - blockPos[i]));
        sideDist[i] = offset * deltaDist[i];
    }

    float traveledDist = 0.0f;
    for (int i = 0; i < 128 && traveledDist <= maxDist; ++i) {
        int chunkX = worldToChunkCoord(blockPos.x, Chunk::chunkWidth);
        int chunkZ = worldToChunkCoord(blockPos.z, Chunk::chunkDepth);
        Chunk* chunk = world->getChunk(chunkX, chunkZ);
        if (chunk) {
            int localX = blockPos.x - chunkX * Chunk::chunkWidth;
            int localY = blockPos.y;
            int localZ = blockPos.z - chunkZ * Chunk::chunkDepth;
            if (localX >= 0 && localX < Chunk::chunkWidth &&
                localY >= 0 && localY < Chunk::chunkHeight &&
                localZ >= 0 && localZ < Chunk::chunkDepth)
            {
                uint8_t type = chunk->blocks[localX][localY][localZ].type;
                if (type != 0) {
                    const BlockDB::BlockInfo* info = BlockDB::getBlockInfo(type);
                    AABB aabb = getModelAABB(type);
                    glm::vec3 boxMin = glm::vec3(blockPos) + aabb.min;
                    glm::vec3 boxMax = glm::vec3(blockPos) + aabb.max;
                    float hitT;
                    if (rayAABBIntersect(origin, dir, boxMin, boxMax, hitT, maxDist)) {
                        result.hit = true;
                        result.hitChunk = chunk;
                        result.hitBlockPos = glm::ivec3(localX, localY, localZ);
                        result.faceNormal = getAABBHitNormal(origin + dir * hitT, boxMin, boxMax);

                        glm::ivec3 placeWorldPos = blockPos + result.faceNormal;
                        int placeChunkX = worldToChunkCoord(placeWorldPos.x, Chunk::chunkWidth);
                        int placeChunkZ = worldToChunkCoord(placeWorldPos.z, Chunk::chunkDepth);
                        Chunk* placeChunk = world->getChunk(placeChunkX, placeChunkZ);
                        if (placeChunk) {
                            int placeLocalX = placeWorldPos.x - placeChunkX * Chunk::chunkWidth;
                            int placeLocalY = placeWorldPos.y;
                            int placeLocalZ = placeWorldPos.z - placeChunkZ * Chunk::chunkDepth;
                            if (placeLocalX >= 0 && placeLocalX < Chunk::chunkWidth &&
                                placeLocalY >= 0 && placeLocalY < Chunk::chunkHeight &&
                                placeLocalZ >= 0 && placeLocalZ < Chunk::chunkDepth)
                            {
                                result.hasPlacePos = true;
                                result.placeBlockPos = glm::ivec3(placeLocalX, placeLocalY, placeLocalZ);
                                result.placeChunk = placeChunk;
                            }
                        }

                        return result;
                    }
                }
            }
        }

        // Step to next voxel
        if (sideDist.x < sideDist.y && sideDist.x < sideDist.z) {
            blockPos.x += step.x;
            traveledDist = sideDist.x;
            sideDist.x += deltaDist.x;
        } else if (sideDist.y < sideDist.z) {
            blockPos.y += step.y;
            traveledDist = sideDist.y;
            sideDist.y += deltaDist.y;
        } else {
            blockPos.z += step.z;
            traveledDist = sideDist.z;
            sideDist.z += deltaDist.z;
        }
    }
    return result;
}

void placeBreakBlockOnClick(World* world, const Camera& camera, char action, uint8_t blockType)
{
    glm::vec3 origin = camera.getPosition();
    glm::vec3 dir = camera.getFront();

    RaycastResult hit = raycast(world, origin, dir, 6.0f);

    int chunkX = 0, chunkZ = 0, x = 0, z = 0;

    // p = place, b = break
    if (action == 'b') {
        if (!hit.hit || !hit.hitChunk) return;
        hit.hitChunk->blocks[hit.hitBlockPos.x][hit.hitBlockPos.y][hit.hitBlockPos.z].type = 0;
        hit.hitChunk->buildMesh();

        chunkX = hit.hitChunk->chunkX;
        chunkZ = hit.hitChunk->chunkZ;
        x = hit.hitBlockPos.x;
        z = hit.hitBlockPos.z;
    }
    else if (action == 'p') {
        if (!hit.hasPlacePos || !hit.placeChunk) return;
        // Prevent placement below bedrock or above chunk height
        if (hit.placeBlockPos.y < 0 || hit.placeBlockPos.y >= Chunk::chunkHeight) return;
        auto& block = hit.placeChunk->blocks[hit.placeBlockPos.x][hit.placeBlockPos.y][hit.placeBlockPos.z];
        if (block.type != 0) return;

        block.type = blockType;
        hit.placeChunk->buildMesh();

        chunkX = hit.placeChunk->chunkX;
        chunkZ = hit.placeChunk->chunkZ;
        x = hit.placeBlockPos.x;
        z = hit.placeBlockPos.z;
    }

    // Rebuild neighbor chunk mesh if at chunk edge
    if (x == 0) {
        Chunk* neighbor = world->getChunk(chunkX - 1, chunkZ);
        if (neighbor) neighbor->buildMesh();
    }
    if (x == Chunk::chunkWidth - 1) {
        Chunk* neighbor = world->getChunk(chunkX + 1, chunkZ);
        if (neighbor) neighbor->buildMesh();
    }
    if (z == 0) {
        Chunk* neighbor = world->getChunk(chunkX, chunkZ - 1);
        if (neighbor) neighbor->buildMesh();
    }
    if (z == Chunk::chunkDepth - 1) {
        Chunk* neighbor = world->getChunk(chunkX, chunkZ + 1);
        if (neighbor) neighbor->buildMesh();
    }
}

// For imgui ----------------------------------------------------------------------------
struct BlockInfo {
    bool valid = false;
    glm::ivec3 worldPos;
    uint8_t type;
};

BlockInfo getLookedAtBlockInfo(World* world, const Camera& camera)
{
    glm::vec3 origin = camera.getPosition();
    glm::vec3 dir = camera.getFront();

    RaycastResult hit = raycast(world, origin, dir, 6.0f);
    if (!hit.hit || !hit.hitChunk) return {};

    BlockInfo info;
    info.valid = true;
    info.worldPos = glm::ivec3(
        hit.hitChunk->chunkX * Chunk::chunkWidth + hit.hitBlockPos.x,
        hit.hitBlockPos.y,
        hit.hitChunk->chunkZ * Chunk::chunkDepth + hit.hitBlockPos.z
    );
    info.type = hit.hitChunk->blocks[hit.hitBlockPos.x][hit.hitBlockPos.y][hit.hitBlockPos.z].type;

    return info;
}