#pragma once

#include <map>
#include "chunk.hpp"

class Chunk;

class World {
public:
    World();
    ~World();

    Chunk* getChunk(int x, int z) const;

    void generateChunks(int radius);
    void render(const Camera& camera, GLint uModelLoc);
    void renderCross(const Camera& camera, GLint uCrossModelLoc);
    void renderLiquid(const Camera& camera, GLint uLiquidModelLoc);

    void updateChunksAroundPlayer(const glm::vec3& playerPos, int radius);

private:
    std::map<std::pair<int, int>, Chunk*> chunks;
    int lastPlayerChunkX = INT32_MIN;
    int lastPlayerChunkZ = INT32_MIN;
};
