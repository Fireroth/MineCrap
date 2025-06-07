#include "chunk.hpp"
#include "options.hpp"
#include <FastNoiseLite.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <cmath>

struct ChunkNoises {
    FastNoiseLite biomeNoise;
    FastNoiseLite baseNoise;
    FastNoiseLite detailNoise;
    FastNoiseLite detail2Noise;
    FastNoiseLite plainsTreeNoise;
    FastNoiseLite forestTreeNoise;
    FastNoiseLite cactusNoise;
};

static ChunkNoises noiseInit() {
    ChunkNoises noises;

    int seed = getOptionInt("world_seed", 1234);

    noises.biomeNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
    noises.biomeNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    noises.biomeNoise.SetFractalOctaves(2);
    noises.biomeNoise.SetFractalWeightedStrength(0.0f);
    noises.biomeNoise.SetFrequency(0.0020f);
    noises.biomeNoise.SetSeed(seed + 1000);

    noises.baseNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noises.baseNoise.SetFrequency(0.005f);
    noises.baseNoise.SetSeed(seed);

    noises.detailNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noises.detailNoise.SetFrequency(0.02f);
    noises.detailNoise.SetSeed(seed + 1);

    noises.detail2Noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noises.detail2Noise.SetFrequency(0.05f);
    noises.detail2Noise.SetSeed(seed + 2);

    noises.plainsTreeNoise.SetNoiseType(FastNoiseLite::NoiseType_Value);
    noises.plainsTreeNoise.SetFrequency(200.0f);
    noises.plainsTreeNoise.SetSeed(seed + 4000);

    noises.forestTreeNoise.SetNoiseType(FastNoiseLite::NoiseType_Value);
    noises.forestTreeNoise.SetFrequency(200.0f);
    noises.forestTreeNoise.SetSeed(seed + 4000);

    noises.cactusNoise.SetNoiseType(FastNoiseLite::NoiseType_Value);
    noises.cactusNoise.SetFrequency(200.0f);
    noises.cactusNoise.SetSeed(seed + 3000);

    return noises;
}

Chunk::Chunk(int x, int z, World* worldPtr)
    : chunkX(x), chunkZ(z), world(worldPtr), VAO(0), VBO(0), EBO(0), indexCount(0) {

    ChunkNoises noises = noiseInit();
    float b = noises.biomeNoise.GetNoise((float)(x * WIDTH), (float)(z * DEPTH));
    if (b >= -1.0f && b < -0.3333f)
        biome = Biome::Forest;
    else if (b >= -0.3333f && b < 0.3333f)
        biome = Biome::Plains;
    else
        biome = Biome::Desert;
    generateTerrain();
}

Chunk::~Chunk() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Chunk::generateTerrain() {
    ChunkNoises noises = noiseInit();

    // PreCompute biome and base heights for all columns in chunk and neighbors
    Biome columnBiomes[WIDTH + 2][DEPTH + 2];
    float columnHeights[WIDTH + 2][DEPTH + 2];

    for (int dx = -1; dx <= WIDTH; ++dx) {
        for (int dz = -1; dz <= DEPTH; ++dz) {
            float fx = static_cast<float>(chunkX * WIDTH + dx);
            float fz = static_cast<float>(chunkZ * DEPTH + dz);
            float b = noises.biomeNoise.GetNoise(fx, fz);
            Biome biome;
            if (b >= -1.0f && b < -0.3333f)
                biome = Biome::Forest;
            else if (b >= -0.3333f && b < 0.3333f)
                biome = Biome::Plains;
            else
                biome = Biome::Desert;

            // Base terrain shape
            float base = noises.baseNoise.GetNoise(fx, fz) * 0.5f + 0.5f;
            float detail = noises.detailNoise.GetNoise(fx, fz) * 0.5f + 0.5f;
            float detail2 = noises.detail2Noise.GetNoise(fx, fz) * 0.5f + 0.5f;

            float heightScale = 1.0f;
            float detailWeight = 1.0f;
            float power = 1.3f;
            float baseHeight = 30.0f;

            switch (biome) {
                case Biome::Desert:
                    heightScale = 0.5f;
                    detailWeight = 0.1f;
                    power = 1.0f;
                    baseHeight = 36.0f;
                    break;
                case Biome::Plains:
                    heightScale = 0.7f;
                    detailWeight = 0.1f;
                    power = 1.0f;
                    baseHeight = 31.0f;
                    break;
                case Biome::Forest:
                    heightScale = 1.0f;
                    detailWeight = 0.4f;
                    power = 1.3f;
                    baseHeight = 30.0f;
                    break;
            }

            float combined = base + detail * detailWeight + detail2 * 0.2f;
            combined = std::pow(combined, power);
            float height = combined * 24.0f * heightScale + baseHeight;

            columnBiomes[dx + 1][dz + 1] = biome;
            columnHeights[dx + 1][dz + 1] = height;
        }
    }

    const int transitionRadius = 4; // blend over 4 blocks

for (int x = 0; x < WIDTH; ++x) {
    for (int z = 0; z < DEPTH; ++z) {
        float totalWeight = 0.0f;
        float blendedHeight = 0.0f;
        std::map<Biome, float> biomeWeights;

        for (int dx = -transitionRadius; dx <= transitionRadius; ++dx) {
            for (int dz = -transitionRadius; dz <= transitionRadius; ++dz) {
                int cx = x + dx + 1;
                int cz = z + dz + 1;

                if (cx < 0 || cz < 0 || cx >= WIDTH + 2 || cz >= DEPTH + 2)
                    continue;

                float dist2 = static_cast<float>(dx * dx + dz * dz);
                float weight = 1.0f / (dist2 + 1.0f);

                Biome b = columnBiomes[cx][cz];
                float h = columnHeights[cx][cz];

                biomeWeights[b] += weight;
                blendedHeight += h * weight;
                totalWeight += weight;
            }
        }

        blendedHeight /= totalWeight;

        Biome finalBiome = Biome::Plains;
        float maxWeight = -1.0f;
        for (auto& [b, w] : biomeWeights) {
            if (w > maxWeight) {
                maxWeight = w;
                finalBiome = b;
            }
        }

        int height = static_cast<int>(blendedHeight);

        for (int y = 0; y < HEIGHT; ++y) {
            if (y == 0) {
                blocks[x][y][z].type = 6; // Bedrock
            } else if (y > height) {
                blocks[x][y][z].type = (y < 37) ? 9 : 0; // Water or air
                continue;
            } else if (y == height) {
                switch (finalBiome) {
                    case Biome::Plains:
                    case Biome::Forest:
                        blocks[x][y][z].type = 1; // Grass
                        break;
                    case Biome::Desert:
                        blocks[x][y][z].type = 4; // Sand
                        break;
                }
            } else if (y >= height - 2) {
                switch (finalBiome) {
                    case Biome::Plains:
                    case Biome::Forest:
                        blocks[x][y][z].type = 2; // Dirt
                        break;
                    case Biome::Desert:
                        blocks[x][y][z].type = 4; // Sand
                        break;
                }
            } else if (y >= height - 4) { // Desert will have stone lower underground
                switch (finalBiome) {
                    case Biome::Plains:
                    case Biome::Forest:
                        blocks[x][y][z].type = 3; // Stone
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
    ChunkNoises noises = noiseInit();
    const Structure* treeStruct = StructureDB::get("tree");
    if (!treeStruct) return;

    for (int x = 2; x < WIDTH - 3; ++x) { // Avoid cut trees because of chunk edges (temporary solution)
        for (int z = 2; z < DEPTH - 3; ++z) {
            float fx = static_cast<float>(chunkX * WIDTH + x);
            float fz = static_cast<float>(chunkZ * DEPTH + z);
            float n = noises.plainsTreeNoise.GetNoise(fx, fz);
            if (n > 0.99f) { // Chance of tree spawning
                int y = HEIGHT - 2;
                while (y > 0 && blocks[x][y][z].type == 0) --y;
                if (blocks[x][y][z].type == 1) {
                    placeStructure(*treeStruct, x - 2, y + 1, z - 2);
                }
            }
        }
    }
}

void Chunk::generateForestFeatures() {
    ChunkNoises noises = noiseInit();
    const Structure* treeStruct = StructureDB::get("tree");
    if (!treeStruct) return;

    for (int x = 2; x < WIDTH - 3; ++x) { // Avoid cut trees because of chunk edges (temporary solution)
        for (int z = 2; z < DEPTH - 3; ++z) {
            float fx = static_cast<float>(chunkX * WIDTH + x);
            float fz = static_cast<float>(chunkZ * DEPTH + z);
            float n = noises.forestTreeNoise.GetNoise(fx, fz);
            if (n > 0.75f) { // Chance of tree spawning
                int y = HEIGHT - 2;
                while (y > 0 && blocks[x][y][z].type == 0) --y;
                if (blocks[x][y][z].type == 1) {
                    placeStructure(*treeStruct, x - 2, y + 1, z - 2);
                }
            }
        }
    }
}

void Chunk::generateDesertFeatures() {
    ChunkNoises noises = noiseInit();
    const Structure* cactusStruct = StructureDB::get("cactus");
    if (!cactusStruct) return;

    for (int x = 0; x < WIDTH; ++x) {
        for (int z = 0; z < DEPTH; ++z) {
            float fx = static_cast<float>(chunkX * WIDTH + x);
            float fz = static_cast<float>(chunkZ * DEPTH + z);
            float n = noises.cactusNoise.GetNoise(fx, fz);
            if (n > 0.95f) { // Chance of cactus spawning
                int y = HEIGHT - 2;
                while (y > 0 && blocks[x][y][z].type == 0) --y;
                if (blocks[x][y][z].type == 4) {
                    placeStructure(*cactusStruct, x, y + 1, z);
                }
            }
        }
    }
}

void Chunk::placeStructure(const Structure& structure, int baseX, int baseY, int baseZ) {
    int structHeight = (int)structure.layers.size();
    if (structHeight == 0) return;
    int structDepth = (int)structure.layers[0].size();
    int structWidth = (int)structure.layers[0][0].size();

    for (int y = 0; y < structHeight; ++y) {
        for (int z = 0; z < structDepth; ++z) {
            for (int x = 0; x < structWidth; ++x) {
                uint8_t blockType = structure.layers[y][z][x];
                if (blockType == 0) continue;
                int wx = baseX + x;
                int wy = baseY + y;
                int wz = baseZ + z;
                if (wx >= 0 && wx < WIDTH && wy >= 0 && wy < HEIGHT && wz >= 0 && wz < DEPTH) {
                    blocks[wx][wy][wz].type = blockType;
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