#pragma once

#include <map>
#include <utility>
#include "chunk.hpp"

class Chunk;

class World {
public:
    World();
    ~World();

    Chunk* getChunk(int x, int z) const;

    void generateChunks(int radius);
    void render(const Camera& camera, GLint uModelLoc);

    void updateChunksAroundPlayer(const glm::vec3& playerPos, int radius);

private:
    std::map<std::pair<int, int>, Chunk*> chunks;
    int lastPlayerChunkX = INT32_MIN;
    int lastPlayerChunkZ = INT32_MIN;
};
