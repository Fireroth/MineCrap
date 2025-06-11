#include <glm/glm.hpp>
#include <iostream>
#include <cmath>
#include <deque>
#include <algorithm>
#include "world.hpp"

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
    int playerChunkX = static_cast<int>(std::floor(playerPos.x / Chunk::WIDTH));
    int playerChunkZ = static_cast<int>(std::floor(playerPos.z / Chunk::DEPTH));

    // Only update if player moved to a new chunk
    if (playerChunkX != lastPlayerChunkX || playerChunkZ != lastPlayerChunkZ) {
        lastPlayerChunkX = playerChunkX;
        lastPlayerChunkZ = playerChunkZ;

        // Unload chunks outside radius
        std::vector<std::pair<int, int>> toRemove;
        for (const auto& [coord, chunk] : chunks) {
            int dx = coord.first - playerChunkX;
            int dz = coord.second - playerChunkZ;
            if (std::abs(dx) > radius || std::abs(dz) > radius) {
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
                int cx = playerChunkX + x;
                int cz = playerChunkZ + z;
                std::pair<int, int> pos = {cx, cz};
                if (chunks.find(pos) == chunks.end()) {
                    positions.push_back(pos);
                }
            }
        }
        std::sort(positions.begin(), positions.end(),
            [playerChunkX, playerChunkZ](const std::pair<int, int>& a, const std::pair<int, int>& b) {
                int da = (a.first - playerChunkX) * (a.first - playerChunkX) + (a.second - playerChunkZ) * (a.second - playerChunkZ);
                int db = (b.first - playerChunkX) * (b.first - playerChunkX) + (b.second - playerChunkZ) * (b.second - playerChunkZ);
                return da < db;
            }
        );
        for (const auto& pos : positions) {
            chunkLoadQueue.push_back(pos);
        }
    }

    if (!chunkLoadQueue.empty()) {
        auto pos = chunkLoadQueue.front();
        chunkLoadQueue.pop_front();
        if (chunks.find(pos) == chunks.end()) {
            Chunk* newChunk = new Chunk(pos.first, pos.second, this);
            chunks[pos] = newChunk;
            newChunk->buildMesh();
            static const int dx[4] = {-1, 1, 0, 0};
            static const int dz[4] = {0, 0, -1, 1};
            for (int i = 0; i < 4; ++i) {
                auto neighbor = getChunk(pos.first + dx[i], pos.second + dz[i]);
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

Chunk* World::getChunk(int x, int z) const {
    auto it = chunks.find({x, z});
    if (it != chunks.end())
        return it->second;
    return nullptr;
}
