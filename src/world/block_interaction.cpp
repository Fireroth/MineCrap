#include <glad/glad.h>
#include "chunk.hpp"
#include "../core/camera.hpp"
#include "world.hpp"

struct RaycastResult {
    bool hit = false;
    glm::ivec3 hitBlockPos;
    Chunk* hitChunk = nullptr;

    bool hasPlacePos = false;
    glm::ivec3 placeBlockPos;
    Chunk* placeChunk = nullptr;

    glm::ivec3 faceNormal = glm::ivec3(0);
};

// DDA raycast
RaycastResult raycast(World* world, const glm::vec3& origin, const glm::vec3& dir, float maxDistance)
{
    RaycastResult result;

    glm::ivec3 blockPos = glm::floor(origin);
    glm::vec3 deltaDist = glm::abs(1.0f / dir);
    glm::ivec3 step = glm::sign(dir);
    glm::vec3 sideDist;

    for (int i = 0; i < 3; ++i) {
        float nextBorder = (step[i] > 0) ? (blockPos[i] + 1.0f) : blockPos[i];
        sideDist[i] = glm::abs(origin[i] - nextBorder) * deltaDist[i];
    }

    float distanceTraveled = 0.0f;
    glm::ivec3 lastBlockPos = blockPos;
    Chunk* lastChunk = nullptr;

    while (distanceTraveled < maxDistance) {
        int cx = blockPos.x >> 4;
        int cz = blockPos.z >> 4;
        Chunk* chunk = world->getChunk(cx, cz);

        if (chunk) {
            int lx = blockPos.x - cx * Chunk::WIDTH;
            int ly = blockPos.y;
            int lz = blockPos.z - cz * Chunk::DEPTH;

            if (lx >= 0 && lx < Chunk::WIDTH &&
                ly >= 0 && ly < Chunk::HEIGHT &&
                lz >= 0 && lz < Chunk::DEPTH &&
                chunk->blocks[lx][ly][lz].type != 0)
            {
                result.hit = true;
                result.hitBlockPos = { lx, ly, lz };
                result.hitChunk = chunk;
                result.faceNormal = blockPos - lastBlockPos;

                if (lastChunk) {
                    result.hasPlacePos = true;
                    result.placeBlockPos = {
                        lastBlockPos.x - lastChunk->chunkX * Chunk::WIDTH,
                        lastBlockPos.y,
                        lastBlockPos.z - lastChunk->chunkZ * Chunk::DEPTH
                    };
                    result.placeChunk = lastChunk;
                }

                return result;
            }
        }

        lastBlockPos = blockPos;
        lastChunk = chunk;

        int axis = (sideDist.x < sideDist.y) ? 
                   ((sideDist.x < sideDist.z) ? 0 : 2) : 
                   ((sideDist.y < sideDist.z) ? 1 : 2);

        sideDist[axis] += deltaDist[axis];
        blockPos[axis] += step[axis];
        distanceTraveled = sideDist[axis] - deltaDist[axis];
    }

    return result;
}

void placeBreakBlockOnClick(World* world, const Camera& camera, char action, uint8_t blockType)
{
    glm::vec3 origin = camera.getPosition();
    glm::vec3 dir = camera.getFront();

    RaycastResult hit = raycast(world, origin, dir, 6.0f);

    int cx = 0, cz = 0, x = 0, z = 0;

    // p = place, b = break
    if (action == 'b') {
        if (!hit.hit || !hit.hitChunk) return;
        hit.hitChunk->blocks[hit.hitBlockPos.x][hit.hitBlockPos.y][hit.hitBlockPos.z].type = 0;
        hit.hitChunk->buildMesh();

        // Assign values for neighbor chunk checks
        cx = hit.hitChunk->chunkX;
        cz = hit.hitChunk->chunkZ;
        x = hit.hitBlockPos.x;
        z = hit.hitBlockPos.z;
    }
    else if (action == 'p') {
        if (!hit.hasPlacePos || !hit.placeChunk) return;
        // Prevent placement below bedrock or above chunk height
        if (hit.placeBlockPos.y < 0 || hit.placeBlockPos.y >= Chunk::HEIGHT) return;
        auto& block = hit.placeChunk->blocks[hit.placeBlockPos.x][hit.placeBlockPos.y][hit.placeBlockPos.z];
        if (block.type != 0) return;

        block.type = blockType;
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
        hit.hitChunk->chunkX * Chunk::WIDTH + hit.hitBlockPos.x,
        hit.hitBlockPos.y,
        hit.hitChunk->chunkZ * Chunk::DEPTH + hit.hitBlockPos.z
    );
    info.type = hit.hitChunk->blocks[hit.hitBlockPos.x][hit.hitBlockPos.y][hit.hitBlockPos.z].type;

    return info;
}