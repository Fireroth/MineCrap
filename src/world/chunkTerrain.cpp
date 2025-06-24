#include <map>
#include "structureDB.hpp"
#include "noise.hpp"
#include "chunkTerrain.hpp"

// Helper function to get biome based on noise value
Chunk::Biome getBiome(float b) {
    if (b >= -1.0f && b < -0.3333f)
        return Chunk::Biome::Forest;
    else if (b >= -0.3333f && b < 0.25f)
        return Chunk::Biome::Plains;
    else
        return Chunk::Biome::Desert;
}

// Helper function to get biome parameters
void getBiomeParams(Chunk::Biome biome, float& heightScale, float& detailWeight, float& power, float& baseHeight) {
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
}

void generateChunkTerrain(Chunk& chunk) {
    const int WIDTH = Chunk::WIDTH;
    const int HEIGHT = Chunk::HEIGHT;
    const int DEPTH = Chunk::DEPTH;
    auto& noises = chunk.noises;
    int chunkX = chunk.chunkX;
    int chunkZ = chunk.chunkZ;
    const int transitionRadius = 5; // blend over 5 blocks

    // Distortion strength for biome edges
    const float biomeDistortStrength = 8.0f;

    // Get the "main" biome for this chunk for feature generation
    float b = noises.biomeNoise.GetNoise(
        (float)(chunkX * WIDTH) + noises.biomeDistortNoise.GetNoise((float)(chunkX * WIDTH), (float)(chunkZ * DEPTH)) * biomeDistortStrength,
        (float)(chunkZ * DEPTH) + noises.biomeDistortNoise.GetNoise((float)(chunkX * WIDTH) + 1000.0f, (float)(chunkZ * DEPTH) + 1000.0f) * biomeDistortStrength
    );
    Chunk::Biome biome = getBiome(b);

    // Precompute biome and height values for the blending to avoid redundant noise calls
    std::vector<std::vector<Chunk::Biome>> biomeCache(WIDTH + 2 * transitionRadius, std::vector<Chunk::Biome>(DEPTH + 2 * transitionRadius));
    std::vector<std::vector<float>> heightCache(WIDTH + 2 * transitionRadius, std::vector<float>(DEPTH + 2 * transitionRadius));

    for (int dx = -transitionRadius; dx < WIDTH + transitionRadius; ++dx) {
        for (int dz = -transitionRadius; dz < DEPTH + transitionRadius; ++dz) {
            int wx = chunkX * WIDTH + dx;
            int wz = chunkZ * DEPTH + dz;

            // Distort biome noise coordinates
            float distortX = noises.biomeDistortNoise.GetNoise((float)wx, (float)wz) * biomeDistortStrength;
            float distortY = noises.biomeDistortNoise.GetNoise((float)wx + 1000.0f, (float)wz + 1000.0f) * biomeDistortStrength;
            float biomeNoise = noises.biomeNoise.GetNoise((float)wx + distortX, (float)wz + distortY);
            Chunk::Biome biome = getBiome(biomeNoise);
            biomeCache[dx + transitionRadius][dz + transitionRadius] = biome;

            float base = noises.baseNoise.GetNoise((float)wx, (float)wz) * 0.5f + 0.5f;
            float detail = noises.detailNoise.GetNoise((float)wx, (float)wz) * 0.5f + 0.5f;
            float detail2 = noises.detail2Noise.GetNoise((float)wx, (float)wz) * 0.5f + 0.5f;

            float heightScale = 1.0f;
            float detailWeight = 1.0f;
            float power = 1.3f;
            float baseHeight = 30.0f;
            getBiomeParams(biome, heightScale, detailWeight, power, baseHeight);

            float combined = base + detail * detailWeight + detail2 * 0.2f;
            combined = std::pow(combined, power);
            float height = combined * 24.0f * heightScale + baseHeight;
            heightCache[dx + transitionRadius][dz + transitionRadius] = height;
        }
    }

    for (int x = 0; x < WIDTH; ++x) {
        for (int z = 0; z < DEPTH; ++z) {
            int wx = chunkX * WIDTH + x;
            int wz = chunkZ * DEPTH + z;

            // Distort biome noise coordinates for this column
            float distortX = noises.biomeDistortNoise.GetNoise((float)wx, (float)wz) * biomeDistortStrength;
            float distortY = noises.biomeDistortNoise.GetNoise((float)wx + 1000.0f, (float)wz + 1000.0f) * biomeDistortStrength;
            float centerBiomeNoise = noises.biomeNoise.GetNoise((float)wx + distortX, (float)wz + distortY);
            Chunk::Biome centerBiome = getBiome(centerBiomeNoise);

            float centerHeight = heightCache[x + transitionRadius][z + transitionRadius];

            // Blending
            bool hasDifferentBiome = false;
            for (int dx = -transitionRadius; dx <= transitionRadius && !hasDifferentBiome; ++dx) {
                for (int dz = -transitionRadius; dz <= transitionRadius && !hasDifferentBiome; ++dz) {
                    Chunk::Biome nBiome = biomeCache[x + dx + transitionRadius][z + dz + transitionRadius];
                    if (nBiome != centerBiome) {
                        hasDifferentBiome = true;
                    }
                }
            }

            float blendedHeight = 0.0f;
            Chunk::Biome finalBiome = centerBiome;

            if (hasDifferentBiome) {
                float totalWeight = 0.0f;
                std::map<Chunk::Biome, float> biomeWeights;

                for (int dx = -transitionRadius; dx <= transitionRadius; ++dx) {
                    for (int dz = -transitionRadius; dz <= transitionRadius; ++dz) {
                        float dist2 = static_cast<float>(dx * dx + dz * dz);
                        float weight = 1.0f / (dist2 + 1.0f);

                        Chunk::Biome nBiome = biomeCache[x + dx + transitionRadius][z + dz + transitionRadius];
                        float nHeight = heightCache[x + dx + transitionRadius][z + dz + transitionRadius];

                        biomeWeights[nBiome] += weight;
                        blendedHeight += nHeight * weight;
                        totalWeight += weight;
                    }
                }

                blendedHeight /= totalWeight;

                float maxWeight = -1.0f;
                for (auto& [b, w] : biomeWeights) {
                    if (w > maxWeight) {
                        maxWeight = w;
                        finalBiome = b;
                    }
                }
            } else {
                // No blending needed
                blendedHeight = centerHeight;
                finalBiome = centerBiome;
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
            //generateChunkBiomeFeatures(chunk, 0, 0.9999f, 19, 19, "big_test", 1);
            generateChunkBiomeFeatures(chunk, 0, 0.998f, 2, 2, "tree", 1);
            break;
        case Chunk::Biome::Forest:
            generateChunkBiomeFeatures(chunk, 0, 0.93f, 2, 2, "tree", 1);
            break;
        case Chunk::Biome::Desert:
            generateChunkBiomeFeatures(chunk, 0, 0.97f, 0, 0, "cactus", 4);
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
