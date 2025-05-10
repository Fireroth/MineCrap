#include <glad/glad.h>
#include "chunk.hpp"
#include "camera.hpp"
#include "world.hpp"
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

struct RaycastResult {
    bool hit = false;
    glm::ivec3 hitBlockPos;
    Chunk* hitChunk = nullptr;

    bool hasPlacePos = false;
    glm::ivec3 placeBlockPos;
    Chunk* placeChunk = nullptr;

    glm::ivec3 faceNormal = glm::ivec3(0);
};

RaycastResult raycast(World* world, const glm::vec3& origin, const glm::vec3& dir, float maxDistance)
{
    RaycastResult result;
    glm::vec3 lastEmptyPos = glm::floor(origin);
    glm::ivec3 lastBlockPos;
    Chunk* lastChunk = nullptr;

    float step = 0.1f;
    for (float d = 0.0f; d < maxDistance; d += step) {
        glm::vec3 checkPos = origin + dir * d;
        glm::vec3 floored = glm::floor(checkPos);

        int cx = static_cast<int>(floored.x) >> 4;
        int cz = static_cast<int>(floored.z) >> 4;
        Chunk* chunk = world->getChunk(cx, cz);
        if (!chunk) continue;

        int lx = static_cast<int>(floored.x) - cx * Chunk::WIDTH;
        int ly = static_cast<int>(floored.y);
        int lz = static_cast<int>(floored.z) - cz * Chunk::DEPTH;

        if (lx < 0 || lx >= Chunk::WIDTH || ly < 0 || ly >= Chunk::HEIGHT || lz < 0 || lz >= Chunk::DEPTH)
            continue;

        if (!chunk->blocks[lx][ly][lz].type.empty()) {
            result.hit = true;
            result.hitBlockPos = glm::ivec3(lx, ly, lz);
            result.hitChunk = chunk;

            glm::ivec3 normal = glm::ivec3(lastEmptyPos - floored);
            result.faceNormal = normal;

            if (lastChunk) {
                result.hasPlacePos = true;
                result.placeBlockPos = lastBlockPos;
                result.placeChunk = lastChunk;
            }

            return result;
        }

        lastEmptyPos = floored;
        lastBlockPos = glm::ivec3(lx, ly, lz);
        lastChunk = chunk;
    }

    return result;
}

void placeBreakBlockOnClick(World* world, const Camera& camera, char action)
{
    glm::vec3 origin = camera.getPosition();
    glm::vec3 dir = camera.getFront();

    RaycastResult hit = raycast(world, origin, dir, 6.0f);

    int cx = 0, cz = 0, x = 0, z = 0;

    // p = place, b = break
    if (action == 'b') {
        if (!hit.hit || !hit.hitChunk) return;
        hit.hitChunk->blocks[hit.hitBlockPos.x][hit.hitBlockPos.y][hit.hitBlockPos.z].type.clear();
        hit.hitChunk->buildMesh();

        // Assign values for neighbor chunk checks
        cx = hit.hitChunk->chunkX;
        cz = hit.hitChunk->chunkZ;
        x = hit.hitBlockPos.x;
        z = hit.hitBlockPos.z;
    }
    else if (action == 'p') {
        if (!hit.hasPlacePos || !hit.placeChunk) return;
        auto& block = hit.placeChunk->blocks[hit.placeBlockPos.x][hit.placeBlockPos.y][hit.placeBlockPos.z];
        if (!block.type.empty()) return;

        block.type = "grass";
        hit.placeChunk->buildMesh();

        // Assign values for neighbor chunk checks
        cx = hit.placeChunk->chunkX;
        cz = hit.placeChunk->chunkZ;
        x = hit.placeBlockPos.x;
        z = hit.placeBlockPos.z;
    }

    // Rebuild neighbor chunk mesh if at chunk edge
    if (x == 0) {
        Chunk* neighbor = world->getChunk(cx - 1, cz);
        if (neighbor) neighbor->buildMesh();
    }
    if (x == Chunk::WIDTH - 1) {
        Chunk* neighbor = world->getChunk(cx + 1, cz);
        if (neighbor) neighbor->buildMesh();
    }
    if (z == 0) {
        Chunk* neighbor = world->getChunk(cx, cz - 1);
        if (neighbor) neighbor->buildMesh();
    }
    if (z == Chunk::DEPTH - 1) {
        Chunk* neighbor = world->getChunk(cx, cz + 1);
        if (neighbor) neighbor->buildMesh();
    }
}

// For imgui ----------------------------------------------------------------------------

struct BlockInfo {
    bool valid = false;
    glm::ivec3 worldPos;
    std::string type;
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
        hit.hitChunk->chunkX * Chunk::WIDTH + hit.hitBlockPos.x,
        hit.hitBlockPos.y,
        hit.hitChunk->chunkZ * Chunk::DEPTH + hit.hitBlockPos.z
    );
    info.type = hit.hitChunk->blocks[hit.hitBlockPos.x][hit.hitBlockPos.y][hit.hitBlockPos.z].type;

    return info;
}