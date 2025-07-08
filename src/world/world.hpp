#pragma once

#include <map>
#include <vector>
#include <glm/glm.hpp>
#include "chunk.hpp"

class Chunk;

struct Frustum {
    glm::vec4 planes[6];
};

class World {
public:
    World();
    ~World();

    Chunk* getChunk(int x, int z) const;

    void generateChunks(int radius);
    void render(const Camera& camera, GLint uModelLoc, const Frustum& frustum);
    void renderCross(const Camera& camera, GLint uCrossModelLoc, const Frustum& frustum);
    void renderLiquid(const Camera& camera, GLint uLiquidModelLoc, const Frustum& frustum);

    void updateChunksAroundPlayer(const glm::vec3& playerPos, int radius);

    static Frustum extractFrustumPlanes(const glm::mat4& projView);
    static bool isChunkInFrustum(int chunkX, int chunkZ, const Frustum& frustum);

private:
    std::map<std::pair<int, int>, Chunk*> chunks;
    int lastPlayerChunkX = INT32_MIN;
    int lastPlayerChunkZ = INT32_MIN;
};
