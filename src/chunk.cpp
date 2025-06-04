#include "chunk.hpp"
#include "options.hpp"
#include <FastNoiseLite.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <cmath>

Chunk::Chunk(int x, int z, World* worldPtr)
    : chunkX(x), chunkZ(z), world(worldPtr), VAO(0), VBO(0), EBO(0), indexCount(0) {
    generateTerrain();
}

Chunk::~Chunk() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Chunk::generateTerrain() {
    FastNoiseLite baseNoise;
    baseNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    baseNoise.SetFrequency(0.005f);
    baseNoise.SetSeed(getOptionInt("world_seed", 1234));

    FastNoiseLite detailNoise;
    detailNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    detailNoise.SetFrequency(0.02f);
    detailNoise.SetSeed(getOptionInt("world_seed", 1234) + 1);

    FastNoiseLite detail2Noise;
    detail2Noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    detail2Noise.SetFrequency(0.05f);
    detail2Noise.SetSeed(getOptionInt("world_seed", 1234) + 2);

    for (int x = 0; x < WIDTH; ++x) {
        for (int z = 0; z < DEPTH; ++z) {
            float fx = static_cast<float>(chunkX * WIDTH + x);
            float fz = static_cast<float>(chunkZ * DEPTH + z);

            // Base terrain shape
            float base = baseNoise.GetNoise(fx, fz) * 0.5f + 0.5f;
            float detail = detailNoise.GetNoise(fx, fz) * 0.5f + 0.5f;
            float detail2 = detailNoise.GetNoise(fx, fz) * 0.5f + 0.5f;

            // Combine with weight
            float combined = (base + detail * 0.3f) + detail2 * 0.2f;
            combined = std::pow(combined, 1.3f);

            int height = static_cast<int>(combined * 24.0f + 30);

            for (int y = 0; y < HEIGHT; ++y) {
                if (y == 0) {
                    blocks[x][y][z].type = 6;
                } else if (y > height) {
                    blocks[x][y][z].type = (y < 40) ? 9 : 0;
                    continue;
                } else if (y == height) {
                    blocks[x][y][z].type = 1;
                } else if (y >= height - 2) {
                    blocks[x][y][z].type = 2;
                } else {
                    blocks[x][y][z].type = 3;
                }
            }
        }
    }
}

void Chunk::buildMesh() {
    // Defer mesh generation if any neighbor chunk is missing
    for (int face = 0; face < 6; ++face) {
        int nx = 0, ny = 0, nz = 0;
        switch (face) {
            case 0: nx = chunkX;     ny = 0; nz = chunkZ + 1; break; // front
            case 1: nx = chunkX;     ny = 0; nz = chunkZ - 1; break; // back
            case 2: nx = chunkX - 1; ny = 0; nz = chunkZ;     break; // left
            case 3: nx = chunkX + 1; ny = 0; nz = chunkZ;     break; // right
            case 4: continue; // top face (no neighbor needed)
            case 5: continue; // bottom face (no neighbor needed)
        }
        if (world->getChunk(nx, nz) == nullptr) {
            // Neighbor chunk missing = skip mesh generation for now
            return;
        }
    }

    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
    }
    if (VBO != 0) {
        glDeleteBuffers(1, &VBO);
        VBO = 0;
    }
    if (EBO != 0) {
        glDeleteBuffers(1, &EBO);
        EBO = 0;
    }
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    unsigned int indexOffset = 0;

    for (int x = 0; x < WIDTH; ++x) {
        for (int y = 0; y < HEIGHT; ++y) {
            for (int z = 0; z < DEPTH; ++z) {
                const uint8_t& type = blocks[x][y][z].type;
                if (type == 0) continue;

                const BlockDB::BlockInfo* info = BlockDB::getBlockInfo(type);
                if (!info) continue;

                for (int face = 0; face < 6; ++face) {
                    if (isBlockVisible(x, y, z, face)) {
                        addFace(vertices, indices, x, y, z, face, info, indexOffset);
                    }
                }
            }
        }
    }

    indexCount = static_cast<GLsizei>(indices.size());

    // Create mesh
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Layout: position (3), uv (2), faceID (1)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

bool Chunk::isBlockVisible(int x, int y, int z, int face) const {
    static const int offsets[6][3] = {
        { 0,  0,  1},  // front
        { 0,  0, -1},  // back
        {-1,  0,  0},  // left
        { 1,  0,  0},  // right
        { 0,  1,  0},  // top
        { 0, -1,  0}   // bottom
    };

    int nx = x + offsets[face][0];
    int ny = y + offsets[face][1];
    int nz = z + offsets[face][2];

    // Check height bounds
    if (ny < 0 || ny >= HEIGHT)
        return true;

    // If neighbor is within current chunk
    if (nx >= 0 && nx < WIDTH && nz >= 0 && nz < DEPTH) {
        return blocks[nx][ny][nz].type == 0;
    }

    // Neighbor is in another chunk
    int neighborChunkX = chunkX;
    int neighborChunkZ = chunkZ;
    int lx = nx;
    int lz = nz;

    if (lx < 0) {
        neighborChunkX -= 1;
        lx += WIDTH;
    } else if (lx >= WIDTH) {
        neighborChunkX += 1;
        lx -= WIDTH;
    }

    if (lz < 0) {
        neighborChunkZ -= 1;
        lz += DEPTH;
    } else if (lz >= DEPTH) {
        neighborChunkZ += 1;
        lz -= DEPTH;
    }

    Chunk* neighbor = world->getChunk(neighborChunkX, neighborChunkZ);
    if (!neighbor)
        return true;  // If no neighbor, assume empty

    return neighbor->blocks[lx][ny][lz].type == 0;
}

void Chunk::addFace(std::vector<float>& vertices, std::vector<unsigned int>& indices,
                    int x, int y, int z, int face, const BlockDB::BlockInfo* blockInfo, unsigned int& indexOffset) {
    static const glm::vec3 faceVertices[6][4] = {
        {{0,0,1}, {1,0,1}, {1,1,1}, {0,1,1}}, // Front
        {{1,0,0}, {0,0,0}, {0,1,0}, {1,1,0}}, // Back
        {{0,0,0}, {0,0,1}, {0,1,1}, {0,1,0}}, // Left
        {{1,0,1}, {1,0,0}, {1,1,0}, {1,1,1}}, // Right
        {{0,1,1}, {1,1,1}, {1,1,0}, {0,1,0}}, // Top
        {{0,0,0}, {1,0,0}, {1,0,1}, {0,0,1}}  // Bottom
    };

    static const glm::vec2 uvs[4] = {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f}
    };

    glm::vec2 texOffset = blockInfo->textureCoords[face] / 16.0f;

    for (int i = 0; i < 4; ++i) {
        glm::vec3 pos = faceVertices[face][i] + glm::vec3(x, y, z);
        glm::vec2 uv = (blockInfo->textureCoords[face] + uvs[i]) / 16.0f;
        vertices.insert(vertices.end(), {pos.x, pos.y, pos.z, uv.x, uv.y, static_cast<float>(face)});
    }

    indices.insert(indices.end(), {
        indexOffset, indexOffset + 1, indexOffset + 2,
        indexOffset + 2, indexOffset + 3, indexOffset
    });

    indexOffset += 4;
}

void Chunk::render(const Camera& camera, GLint uModelLoc) {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(chunkX * WIDTH, 0, chunkZ * DEPTH));
    glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, &model[0][0]);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}