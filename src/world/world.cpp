#include <glm/gtc/matrix_access.hpp>
#include <iostream>
#include <cmath>
#include <deque>
#include <algorithm>
#include "world.hpp"
#include "../core/options.hpp"

static std::deque<std::pair<int, int>> chunkLoadQueue;

World::World() {}

World::~World() {
    for (auto& [coord, chunk] : chunks) {
        delete chunk;
    }
    chunks.clear();
}

void World::generateChunks(int radius) {
    // Create chunks
    for (int x = -radius; x <= radius; x++) {
        for (int z = -radius; z <= radius; z++) {
            std::pair<int, int> pos = {x, z};
            if (chunks.find(pos) == chunks.end()) {
                chunks[pos] = new Chunk(x, z, this);
            }
        }
    }

    // Build meshes
    for (auto& [coord, chunk] : chunks) {
        chunk->buildMesh();
    }
}

void World::updateChunksAroundPlayer(const glm::vec3& playerPos, int radius, bool force) {
    int playerChunkX = static_cast<int>(std::floor(playerPos.x / Chunk::chunkWidth));
    int playerChunkZ = static_cast<int>(std::floor(playerPos.z / Chunk::chunkDepth));

    // Only update if player moved to a new chunk
    if (playerChunkX != lastPlayerChunkX || playerChunkZ != lastPlayerChunkZ || force) {
        lastPlayerChunkX = playerChunkX;
        lastPlayerChunkZ = playerChunkZ;

        // Unload chunks outside radius
        for (auto iterator = chunks.begin(); iterator != chunks.end();) {
            int chunkOffsetX = iterator->first.first - playerChunkX;
            int chunkOffsetZ = iterator->first.second - playerChunkZ;
            if (std::abs(chunkOffsetX) > radius || std::abs(chunkOffsetZ) > radius) {
                delete iterator->second;
                iterator = chunks.erase(iterator);
            } else {
                iterator++;
            }
        }

        chunkLoadQueue.clear();
        std::vector<std::pair<int, int>> positions;
        for (int x = -radius; x <= radius; x++) {
            for (int z = -radius; z <= radius; z++) {
                int chunkX = playerChunkX + x;
                int chunkZ = playerChunkZ + z;
                std::pair<int, int> pos = {chunkX, chunkZ};
                if (chunks.find(pos) == chunks.end()) {
                    positions.push_back(pos);
                }
            }
        }
        std::sort(positions.begin(), positions.end(),
            [playerChunkX, playerChunkZ](const std::pair<int, int>& a, const std::pair<int, int>& b) {
                int distanceA = (a.first - playerChunkX) * (a.first - playerChunkX) + (a.second - playerChunkZ) * (a.second - playerChunkZ);
                int distanceB = (b.first - playerChunkX) * (b.first - playerChunkX) + (b.second - playerChunkZ) * (b.second - playerChunkZ);
                return distanceA < distanceB;
            }
        );
        for (const auto& pos : positions) {
            chunkLoadQueue.push_back(pos);
        }
    }

    static int chunksToLoadPerFrame = getOptionInt("chunks_to_load_per_frame", 1);
    for (int i = 0; i < chunksToLoadPerFrame && !chunkLoadQueue.empty(); i++) {
        auto pos = chunkLoadQueue.front();
        chunkLoadQueue.pop_front();
        Chunk* newChunk = new Chunk(pos.first, pos.second, this);
        chunks[pos] = newChunk;
        newChunk->buildMesh();
        static const int neighborChunkOffsetX[4] = {-1, 1, 0, 0};
        static const int neighborChunkOffsetZ[4] = {0, 0, -1, 1};
        for (int i = 0; i < 4; i++) {
            auto neighbor = getChunk(pos.first + neighborChunkOffsetX[i], pos.second + neighborChunkOffsetZ[i]);
            if (neighbor) neighbor->buildMesh();  
        }
    }
}

Frustum World::extractFrustumPlanes(const glm::mat4& projView) {
    Frustum frustum;
    frustum.planes[0] = glm::row(projView, 3) + glm::row(projView, 0); // Left
    frustum.planes[1] = glm::row(projView, 3) - glm::row(projView, 0); // Right
    frustum.planes[2] = glm::row(projView, 3) + glm::row(projView, 1); // Bottom
    frustum.planes[3] = glm::row(projView, 3) - glm::row(projView, 1); // Top
    frustum.planes[4] = glm::row(projView, 3) + glm::row(projView, 2); // Near
    frustum.planes[5] = glm::row(projView, 3) - glm::row(projView, 2); // Far
    return frustum;
}

bool World::isChunkInFrustum(int chunkX, int chunkZ, const Frustum& frustum) {
    float minX = static_cast<float>(chunkX * Chunk::chunkWidth);
    float maxX = minX + static_cast<float>(Chunk::chunkWidth);
    float minY = 0.0f;
    float maxY = static_cast<float>(Chunk::chunkHeight);
    float minZ = static_cast<float>(chunkZ * Chunk::chunkDepth);
    float maxZ = minZ + static_cast<float>(Chunk::chunkDepth);

    for (int i = 0; i < 6; i++) {
        const glm::vec4& plane = frustum.planes[i];
        int out = 0;
        out += (glm::dot(plane, glm::vec4(minX, minY, minZ, 1.0f)) < 0.0f) ? 1 : 0;
        out += (glm::dot(plane, glm::vec4(maxX, minY, minZ, 1.0f)) < 0.0f) ? 1 : 0;
        out += (glm::dot(plane, glm::vec4(minX, maxY, minZ, 1.0f)) < 0.0f) ? 1 : 0;
        out += (glm::dot(plane, glm::vec4(maxX, maxY, minZ, 1.0f)) < 0.0f) ? 1 : 0;
        out += (glm::dot(plane, glm::vec4(minX, minY, maxZ, 1.0f)) < 0.0f) ? 1 : 0;
        out += (glm::dot(plane, glm::vec4(maxX, minY, maxZ, 1.0f)) < 0.0f) ? 1 : 0;
        out += (glm::dot(plane, glm::vec4(minX, maxY, maxZ, 1.0f)) < 0.0f) ? 1 : 0;
        out += (glm::dot(plane, glm::vec4(maxX, maxY, maxZ, 1.0f)) < 0.0f) ? 1 : 0;
        if (out == 8) return false;
    }
    return true;
}

void World::render(const Camera& camera, GLint uModelLoc, const Frustum& frustum) {
    for (auto& [coord, chunk] : chunks) {
        if (isChunkInFrustum(coord.first, coord.second, frustum))
            chunk->render(camera, uModelLoc);
    }
}

void World::renderCross(const Camera& camera, GLint uCrossModelLoc, const Frustum& frustum) {
    for (auto& [coord, chunk] : chunks) {
        if (isChunkInFrustum(coord.first, coord.second, frustum))
            chunk->renderCross(camera, uCrossModelLoc);
    }
}
void World::renderLiquid(const Camera& camera, GLint uLiquidModelLoc, const Frustum& frustum) {
    std::vector<std::pair<float, Chunk*>> visible;
    visible.reserve(chunks.size());

    glm::vec3 camPos = camera.getPosition();
    for (auto& [coord, chunk] : chunks) {
        if (!isChunkInFrustum(coord.first, coord.second, frustum))
            continue;

        float cx = (coord.first * Chunk::chunkWidth) + (Chunk::chunkWidth * 0.5f);
        float cz = (coord.second * Chunk::chunkDepth) + (Chunk::chunkDepth * 0.5f);
        float dx = camPos.x - cx;
        float dy = camPos.y;
        float dz = camPos.z - cz;
        float dist2 = dx*dx + dy*dy + dz*dz;
        visible.emplace_back(dist2, chunk);
    }

    std::sort(visible.begin(), visible.end(), [](const auto& A, const auto& B) {
        return A.first > B.first;
    });

    for (auto& p : visible) {
        p.second->renderLiquid(camera, uLiquidModelLoc);
    }
}

Chunk* World::getChunk(int x, int z) const {
    auto iterator = chunks.find({x, z});
    if (iterator != chunks.end())
        return iterator->second;
    return nullptr;
}
