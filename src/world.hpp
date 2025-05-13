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

private:
    std::map<std::pair<int, int>, Chunk*> chunks;
};
