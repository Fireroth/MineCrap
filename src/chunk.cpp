#include "chunk.hpp"
#include "options.hpp"
#include <FastNoiseLite.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <cmath>

Chunk::Chunk(int x, int z, World* worldPtr)
    : chunkX(x), chunkZ(z), world(worldPtr), VAO(0), VBO(0), EBO(0), indexCount(0) {

    FastNoiseLite biomeNoise;
    biomeNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    biomeNoise.SetFrequency(0.0015f);
    biomeNoise.SetSeed(getOptionInt("world_seed", 1234) + 1000);
    float b = biomeNoise.GetNoise((float)(x * WIDTH), (float)(z * DEPTH));
    if (b >= 0.5f)
        biome = Biome::Forest;
    else if (b >= 0.0f)
        biome = Biome::Desert;
    else
        biome = Biome::Plains;
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

    FastNoiseLite biomeNoise;
    biomeNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    biomeNoise.SetFrequency(0.0015f);
    biomeNoise.SetSeed(getOptionInt("world_seed", 1234) + 1000);

    for (int x = 0; x < WIDTH; ++x) {
        for (int z = 0; z < DEPTH; ++z) {
            float fx = static_cast<float>(chunkX * WIDTH + x);
            float fz = static_cast<float>(chunkZ * DEPTH + z);

            // Base terrain shape
            float base = baseNoise.GetNoise(fx, fz) * 0.5f + 0.5f;
            float detail = detailNoise.GetNoise(fx, fz) * 0.5f + 0.5f;
            float detail2 = detail2Noise.GetNoise(fx, fz) * 0.5f + 0.5f;

            // Smooth biome transition
            float biomeVal = biomeNoise.GetNoise(fx, fz); // -1..1
            float tDesert = glm::clamp(0.5f - biomeVal, 0.0f, 1.0f); // 1 at -1, 0 at 0.5
            float tPlains = glm::clamp(biomeVal, 0.0f, 0.5f) * 2.0f; // 1 at 0.25, 0 at 0 or 0.5
            float tForest = glm::clamp(biomeVal - 0.5f, 0.0f, 0.5f) * 2.0f; // 1 at 1, 0 at 0.5

            // Biome terrain modifiers
            float biomeMod = 0.7f * tDesert + 0.6f * tPlains + 1.1f * tForest;

            float combined = (base + detail * 0.3f) + detail2 * 0.2f;
            combined = std::pow(combined, 1.3f);

            int height = static_cast<int>(combined * 24.0f * biomeMod + 30);

            Biome columnBiome;
            if (biomeVal >= 0.5f)
                columnBiome = Biome::Forest;
            else if (biomeVal >= 0.0f)
                columnBiome = Biome::Desert;
            else
                columnBiome = Biome::Plains;

            for (int y = 0; y < HEIGHT; ++y) {
                if (y == 0) {
                    blocks[x][y][z].type = 6; // Bedrock
                } else if (y > height) {
                    blocks[x][y][z].type = (y < 37) ? 9 : 0; // Water or air
                    continue;
                } else if (y == height) {
                    switch (columnBiome) {
                        case Biome::Plains:
                        case Biome::Forest:
                            blocks[x][y][z].type = 1; // Grass
                            break;
                        case Biome::Desert:
                            blocks[x][y][z].type = 4; // Sand
                            break;
                    }
                } else if (y >= height - 2) {
                    switch (columnBiome) {
                        case Biome::Plains:
                        case Biome::Forest:
                            blocks[x][y][z].type = 2; // Dirt
                            break;
                        case Biome::Desert:
                            blocks[x][y][z].type = 4; // Sand
                            break;
                    }
                } else {
                    blocks[x][y][z].type = 3; // Stone
                }
            }
        }
    }

    // Biome-specific features
    switch (biome) {
        case Biome::Plains:
            generatePlainsFeatures();
            break;
        case Biome::Forest:
            generateForestFeatures();
            break;
        case Biome::Desert:
            generateDesertFeatures();
            break;
    }
}

void Chunk::generatePlainsFeatures() {
    FastNoiseLite treeNoise;
    treeNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    treeNoise.SetFrequency(200.0f);
    treeNoise.SetSeed(getOptionInt("world_seed", 1234) + 4000);

    for (int x = 2; x < WIDTH - 2; ++x) { // Avoid cut trees because of chunk edges (temporary solution)
        for (int z = 2; z < DEPTH - 2; ++z) {
            float fx = static_cast<float>(chunkX * WIDTH + x);
            float fz = static_cast<float>(chunkZ * DEPTH + z);
            float n = treeNoise.GetNoise(fx, fz);
            if (n > 0.97f) { // Chance of tree spawning
                int y = HEIGHT - 2;
                while (y > 0 && blocks[x][y][z].type == 0) --y;
                if (blocks[x][y][z].type == 1) { // Only on grass
                    int trunkHeight = 4 + (int)((n - 0.4f) * 4);
                    for (int t = 1; t <= trunkHeight && y + t < HEIGHT; ++t)
                        blocks[x][y + t][z].type = 5;
                    for (int dx = -2; dx <= 2; ++dx)
                        for (int dz = -2; dz <= 2; ++dz)
                            for (int dy = trunkHeight - 1; dy <= trunkHeight + 1; ++dy) {
                                int lx = x + dx, ly = y + dy, lz = z + dz;
                                if (lx >= 0 && lx < WIDTH && lz >= 0 && lz < DEPTH && ly >= 0 && ly < HEIGHT) {
                                    if (std::abs(dx) + std::abs(dz) + std::abs(dy - trunkHeight) < 4)
                                        if (blocks[lx][ly][lz].type == 0)
                                            blocks[lx][ly][lz].type = 11;
                                }
                            }
                }
            }
        }
    }
}

void Chunk::generateForestFeatures() {
    FastNoiseLite treeNoise;
    treeNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    treeNoise.SetFrequency(100.0f);
    treeNoise.SetSeed(getOptionInt("world_seed", 1234) + 4000);

    for (int x = 2; x < WIDTH - 2; ++x) { // Avoid cut trees because of chunk edges (temporary solution)
        for (int z = 2; z < DEPTH - 2; ++z) {
            float fx = static_cast<float>(chunkX * WIDTH + x);
            float fz = static_cast<float>(chunkZ * DEPTH + z);
            float n = treeNoise.GetNoise(fx, fz);
            if (n > 0.7f) { // Chance of tree spawning
                int y = HEIGHT - 2;
                while (y > 0 && blocks[x][y][z].type == 0) --y;
                if (blocks[x][y][z].type == 1) { // Only on grass
                    int trunkHeight = 4 + (int)((n - 0.4f) * 4);
                    for (int t = 1; t <= trunkHeight && y + t < HEIGHT; ++t)
                        blocks[x][y + t][z].type = 5;
                    for (int dx = -2; dx <= 2; ++dx)
                        for (int dz = -2; dz <= 2; ++dz)
                            for (int dy = trunkHeight - 1; dy <= trunkHeight + 1; ++dy) {
                                int lx = x + dx, ly = y + dy, lz = z + dz;
                                if (lx >= 0 && lx < WIDTH && lz >= 0 && lz < DEPTH && ly >= 0 && ly < HEIGHT) {
                                    if (std::abs(dx) + std::abs(dz) + std::abs(dy - trunkHeight) < 4)
                                        if (blocks[lx][ly][lz].type == 0)
                                            blocks[lx][ly][lz].type = 11;
                                }
                            }
                }
            }
        }
    }
}

void Chunk::generateDesertFeatures() {
    FastNoiseLite cactusNoise;
    cactusNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    cactusNoise.SetFrequency(100.0f);
    cactusNoise.SetSeed(getOptionInt("world_seed", 1234) + 3000);

    for (int x = 1; x < WIDTH - 1; ++x) {
        for (int z = 1; z < DEPTH - 1; ++z) {
            float fx = static_cast<float>(chunkX * WIDTH + x);
            float fz = static_cast<float>(chunkZ * DEPTH + z);
            float n = cactusNoise.GetNoise(fx, fz);
            if (n > 0.85f) { // Chance of cactus spawning
                int y = HEIGHT - 2;
                while (y > 0 && blocks[x][y][z].type == 0) --y;
                if (blocks[x][y][z].type == 4) { // Only on sand
                    // Cactus height: random between 2 and 4
                    int cactusHeight = 2 + static_cast<int>((n - 0.85f) / (1.0f - 0.85f) * 2.99f);
                    if (cactusHeight > 4) cactusHeight = 4;
                    if (cactusHeight < 2) cactusHeight = 2;
                    for (int t = 1; t <= cactusHeight && y + t < HEIGHT; ++t)
                        blocks[x][y + t][z].type = 12; // Cactus
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