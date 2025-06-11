#include <map>
#include "structureDB.hpp"
#include "noise.hpp"
#include "chunkTerrain.hpp"

Chunk::Biome getBiome(float b, Chunk& chunk) {
    if (b >= -1.0f && b < -0.3333f)
        chunk.biome = Chunk::Biome::Forest;
    else if (b >= -0.3333f && b < 0.3333f)
        chunk.biome = Chunk::Biome::Plains;
    else
        chunk.biome = Chunk::Biome::Desert;
    return chunk.biome;
}

void generateChunkTerrain(Chunk& chunk) {
    const int WIDTH = Chunk::WIDTH;
    const int HEIGHT = Chunk::HEIGHT;
    const int DEPTH = Chunk::DEPTH;
    auto& noises = chunk.noises;
    int chunkX = chunk.chunkX;
    int chunkZ = chunk.chunkZ;

    float b = noises.biomeNoise.GetNoise((float)(chunkX * WIDTH), (float)(chunkZ * DEPTH));
    Chunk::Biome biome = getBiome(b, chunk);

    // PreCompute biome and base heights for all columns in chunk and neighbors
    Chunk::Biome columnBiomes[WIDTH + 2][DEPTH + 2];
    float columnHeights[WIDTH + 2][DEPTH + 2];

    for (int dx = -1; dx <= WIDTH; ++dx) {
        for (int dz = -1; dz <= DEPTH; ++dz) {
            float fx = static_cast<float>(chunkX * WIDTH + dx);
            float fz = static_cast<float>(chunkZ * DEPTH + dz);
            float b = noises.biomeNoise.GetNoise(fx, fz);
            Chunk::Biome biome = getBiome(b, chunk);

            // Base terrain shape
            float base = noises.baseNoise.GetNoise(fx, fz) * 0.5f + 0.5f;
            float detail = noises.detailNoise.GetNoise(fx, fz) * 0.5f + 0.5f;
            float detail2 = noises.detail2Noise.GetNoise(fx, fz) * 0.5f + 0.5f;

            float heightScale = 1.0f;
            float detailWeight = 1.0f;
            float power = 1.3f;
            float baseHeight = 30.0f;

            switch (biome) {
                case Chunk::Biome::Desert:
                    heightScale = 0.5f;
                    detailWeight = 0.1f;
                    power = 1.0f;
                    baseHeight = 36.0f;
                    break;
                case Chunk::Biome::Plains:
                    heightScale = 0.7f;
                    detailWeight = 0.1f;
                    power = 1.0f;
                    baseHeight = 31.0f;
                    break;
                case Chunk::Biome::Forest:
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
            std::map<Chunk::Biome, float> biomeWeights;

            for (int dx = -transitionRadius; dx <= transitionRadius; ++dx) {
                for (int dz = -transitionRadius; dz <= transitionRadius; ++dz) {
                    int cx = x + dx + 1;
                    int cz = z + dz + 1;

                    if (cx < 0 || cz < 0 || cx >= WIDTH + 2 || cz >= DEPTH + 2)
                        continue;

                    float dist2 = static_cast<float>(dx * dx + dz * dz);
                    float weight = 1.0f / (dist2 + 1.0f);

                    Chunk::Biome b = columnBiomes[cx][cz];
                    float h = columnHeights[cx][cz];

                    biomeWeights[b] += weight;
                    blendedHeight += h * weight;
                    totalWeight += weight;
                }
            }

            blendedHeight /= totalWeight;

            Chunk::Biome finalBiome = Chunk::Biome::Plains;
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
                    chunk.blocks[x][y][z].type = 6; // Bedrock
                } else if (y > height) {
                    chunk.blocks[x][y][z].type = (y < 37) ? 9 : 0; // Water or air
                    continue;
                } else if (y == height) {
                    switch (finalBiome) {
                        case Chunk::Biome::Plains:
                        case Chunk::Biome::Forest:
                            chunk.blocks[x][y][z].type = 1; // Grass
                            break;
                        case Chunk::Biome::Desert:
                            chunk.blocks[x][y][z].type = 4; // Sand
                            break;
                    }
                } else if (y >= height - 2) {
                    switch (finalBiome) {
                        case Chunk::Biome::Plains:
                        case Chunk::Biome::Forest:
                            chunk.blocks[x][y][z].type = 2; // Dirt
                            break;
                        case Chunk::Biome::Desert:
                            chunk.blocks[x][y][z].type = 4; // Sand
                            break;
                    }
                } else if (y >= height - 4) { // Desert will have stone lower underground
                    switch (finalBiome) {
                        case Chunk::Biome::Plains:
                        case Chunk::Biome::Forest:
                            chunk.blocks[x][y][z].type = 3; // Stone
                            break;
                        case Chunk::Biome::Desert:
                            chunk.blocks[x][y][z].type = 4; // Sand
                            break;
                    }
                } else {
                    chunk.blocks[x][y][z].type = 3; // Stone
                }
            }
        }
    }

    // Biome specific features
    chunk.biome = biome;
    switch (biome) {
        case Chunk::Biome::Plains:
            generateChunkBiomeFeatures(chunk, 0, 0.995f, 2, 2, "tree", 1);
            break;
        case Chunk::Biome::Forest:
            generateChunkBiomeFeatures(chunk, 0, 0.95f, 2, 2, "tree", 1);
            break;
        case Chunk::Biome::Desert:
            generateChunkBiomeFeatures(chunk, 0, 0.95f, 0, 0, "cactus", 4);
            break;
    }
}

void generateChunkBiomeFeatures(Chunk& chunk, int margin, float treshold, int xOffset, int zOffset, std::string structureName, int allowedBlockID) {
    ChunkNoises noises = noiseInit();
    const Structure* structure = StructureDB::get(structureName);
    if (!structure) return;

    for (int x = margin; x < Chunk::WIDTH - margin; ++x) {
        for (int z = margin; z < Chunk::DEPTH - margin; ++z) {
            float fx = static_cast<float>(chunk.chunkX * Chunk::WIDTH + x);
            float fz = static_cast<float>(chunk.chunkZ * Chunk::DEPTH + z);
            float n = noises.featureNoise.GetNoise(fx, fz);
            if (n > treshold) { // Chance of feature spawning
                int y = Chunk::HEIGHT - 2;
                while (y > 0 && chunk.blocks[x][y][z].type == 0) --y; {
                    if (chunk.blocks[x][y][z].type == allowedBlockID) {
                        chunk.placeStructure(*structure, x - xOffset, y + 1, z - zOffset);
                    }
                }
            }
        }
    }
}
