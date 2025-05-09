#include "world.hpp"
#include <glm/glm.hpp>
#include <iostream>

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


void World::render(const Camera& camera, GLuint shaderProgram) {
    for (auto& [coord, chunk] : chunks) {
        chunk->render(camera, shaderProgram);
    }
}

Chunk* World::getChunk(int x, int z) const {
    auto it = chunks.find({x, z});
    if (it != chunks.end())
        return it->second;
    return nullptr;
}


