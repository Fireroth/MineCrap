#include <glm/glm.hpp>
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
    for (int x = -radius; x <= radius; ++x) {
        for (int z = -radius; z <= radius; ++z) {
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

void World::updateChunksAroundPlayer(const glm::vec3& playerPos, int radius) {
    int playerChunkX = static_cast<int>(std::floor(playerPos.x / Chunk::chunkWidth));
    int playerChunkZ = static_cast<int>(std::floor(playerPos.z / Chunk::chunkDepth));

    // Only update if player moved to a new chunk
    if (playerChunkX != lastPlayerChunkX || playerChunkZ != lastPlayerChunkZ) {
        lastPlayerChunkX = playerChunkX;
        lastPlayerChunkZ = playerChunkZ;

        // Unload chunks outside radius
        std::vector<std::pair<int, int>> toRemove;
        for (const auto& [coord, chunk] : chunks) {
            int chunkOffsetX = coord.first - playerChunkX;
            int chunkOffsetZ = coord.second - playerChunkZ;
            if (std::abs(chunkOffsetX) > radius || std::abs(chunkOffsetZ) > radius) {
                toRemove.push_back(coord);
            }
        }
        for (const auto& coord : toRemove) {
            delete chunks[coord];
            chunks.erase(coord);
        }

        chunkLoadQueue.clear();
        std::vector<std::pair<int, int>> positions;
        for (int x = -radius; x <= radius; ++x) {
            for (int z = -radius; z <= radius; ++z) {
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
    for (int i = 0; i < chunksToLoadPerFrame && !chunkLoadQueue.empty(); ++i) {
        auto pos = chunkLoadQueue.front();
        chunkLoadQueue.pop_front();
        if (chunks.find(pos) == chunks.end()) {
            Chunk* newChunk = new Chunk(pos.first, pos.second, this);
            chunks[pos] = newChunk;
            newChunk->buildMesh();
            static const int neighborChunkOffsetX[4] = {-1, 1, 0, 0};
            static const int neighborChunkOffsetZ[4] = {0, 0, -1, 1};
            for (int i = 0; i < 4; ++i) {
                auto neighbor = getChunk(pos.first + neighborChunkOffsetX[i], pos.second + neighborChunkOffsetZ[i]);
                if (neighbor) neighbor->buildMesh();
            }
        }
    }
}

void World::render(const Camera& camera, GLint uModelLoc) {
    for (auto& [coord, chunk] : chunks) {
        chunk->render(camera, uModelLoc);
    }
}

void World::renderCross(const Camera& camera, GLint uCrossModelLoc) {
    for (auto& [coord, chunk] : chunks) {
        chunk->renderCross(camera, uCrossModelLoc);
    }
}

void World::renderLiquid(const Camera& camera, GLint uLiquidModelLoc) {
    for (auto& [coord, chunk] : chunks) {
        chunk->renderLiquid(camera, uLiquidModelLoc);
    }
}

Chunk* World::getChunk(int x, int z) const {
    auto iterator = chunks.find({x, z});
    if (iterator != chunks.end())
        return iterator->second;
    return nullptr;
}
