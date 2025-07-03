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
    const int transitionRadius = 5; // blend over 5 blocks
    const float biomeDistortStrength = 8.0f;

    const int chunkWidth = Chunk::chunkWidth;
    const int chunkHeight = Chunk::chunkHeight;
    const int chunkDepth = Chunk::chunkDepth;
    auto& noises = chunk.noises;
    int chunkX = chunk.chunkX;
    int chunkZ = chunk.chunkZ;
    
    // Get the "main" biome for this chunk for feature generation
    float b = noises.biomeNoise.GetNoise(
        (float)(chunkX * chunkWidth) + noises.biomeDistortNoise.GetNoise((float)(chunkX * chunkWidth), (float)(chunkZ * chunkDepth)) * biomeDistortStrength,
        (float)(chunkZ * chunkDepth) + noises.biomeDistortNoise.GetNoise((float)(chunkX * chunkWidth) + 1000.0f, (float)(chunkZ * chunkDepth) + 1000.0f) * biomeDistortStrength
    );
    Chunk::Biome biome = getBiome(b);

    // Precompute biome and height values for the blending
    std::vector<std::vector<Chunk::Biome>> biomeCache(chunkWidth + 2 * transitionRadius, std::vector<Chunk::Biome>(chunkDepth + 2 * transitionRadius));
    std::vector<std::vector<float>> heightCache(chunkWidth + 2 * transitionRadius, std::vector<float>(chunkDepth + 2 * transitionRadius));

    for (int localOffsetX = -transitionRadius; localOffsetX < chunkWidth + transitionRadius; ++localOffsetX) {
        for (int localOffsetZ = -transitionRadius; localOffsetZ < chunkDepth + transitionRadius; ++localOffsetZ) {
            int worldX = chunkX * chunkWidth + localOffsetX;
            int worldZ = chunkZ * chunkDepth + localOffsetZ;

            // Distort biome noise coordinates
            float distortX = noises.biomeDistortNoise.GetNoise((float)worldX, (float)worldZ) * biomeDistortStrength;
            float distortY = noises.biomeDistortNoise.GetNoise((float)worldX + 1000.0f, (float)worldZ + 1000.0f) * biomeDistortStrength;
            float biomeNoise = noises.biomeNoise.GetNoise((float)worldX + distortX, (float)worldZ + distortY);
            Chunk::Biome biome = getBiome(biomeNoise);
            biomeCache[localOffsetX + transitionRadius][localOffsetZ + transitionRadius] = biome;

            float base = noises.baseNoise.GetNoise((float)worldX, (float)worldZ) * 0.5f + 0.5f;
            float detail = noises.detailNoise.GetNoise((float)worldX, (float)worldZ) * 0.5f + 0.5f;
            float detail2 = noises.detail2Noise.GetNoise((float)worldX, (float)worldZ) * 0.5f + 0.5f;

            float heightScale = 1.0f;
            float detailWeight = 1.0f;
            float power = 1.3f;
            float baseHeight = 30.0f;
            getBiomeParams(biome, heightScale, detailWeight, power, baseHeight);

            float combined = base + detail * detailWeight + detail2 * 0.2f;
            combined = std::pow(combined, power);
            float height = combined * 24.0f * heightScale + baseHeight;
            heightCache[localOffsetX + transitionRadius][localOffsetZ + transitionRadius] = height;
        }
    }

    for (int x = 0; x < chunkWidth; ++x) {
        for (int z = 0; z < chunkDepth; ++z) {
            int worldX = chunkX * chunkWidth + x;
            int worldZ = chunkZ * chunkDepth + z;

            // Distort biome noise coordinates for this column
            float distortX = noises.biomeDistortNoise.GetNoise((float)worldX, (float)worldZ) * biomeDistortStrength;
            float distortY = noises.biomeDistortNoise.GetNoise((float)worldX + 1000.0f, (float)worldZ + 1000.0f) * biomeDistortStrength;
            float centerBiomeNoise = noises.biomeNoise.GetNoise((float)worldX + distortX, (float)worldZ + distortY);
            Chunk::Biome centerBiome = getBiome(centerBiomeNoise);

            float centerHeight = heightCache[x + transitionRadius][z + transitionRadius];

            // Blending
            bool hasDifferentBiome = false;
            for (int localOffsetX = -transitionRadius; localOffsetX <= transitionRadius && !hasDifferentBiome; ++localOffsetX) {
                for (int localOffsetZ = -transitionRadius; localOffsetZ <= transitionRadius && !hasDifferentBiome; ++localOffsetZ) {
                    Chunk::Biome nBiome = biomeCache[x + localOffsetX + transitionRadius][z + localOffsetZ + transitionRadius];
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

                for (int localOffsetX = -transitionRadius; localOffsetX <= transitionRadius; ++localOffsetX) {
                    for (int localOffsetZ = -transitionRadius; localOffsetZ <= transitionRadius; ++localOffsetZ) {
                        float dist2 = static_cast<float>(localOffsetX * localOffsetX + localOffsetZ * localOffsetZ);
                        float weight = 1.0f / (dist2 + 1.0f);

                        Chunk::Biome nBiome = biomeCache[x + localOffsetX + transitionRadius][z + localOffsetZ + transitionRadius];
                        float nHeight = heightCache[x + localOffsetX + transitionRadius][z + localOffsetZ + transitionRadius];

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

            for (int y = 0; y < chunkHeight; ++y) {
                if (y == 0) {
                    chunk.blocks[x][y][z].type = 6; // Bedrock
                } else if (y > height) {
                    chunk.blocks[x][y][z].type = (y < 37) ? 9 : 0; // Water or air
                    continue;
                } else if (y == height) {
                    switch (finalBiome) {
                        case Chunk::Biome::Plains:
                        case Chunk::Biome::Forest:
                            // If grass would be generated below y 36, use sand instead
                            chunk.blocks[x][y][z].type = (y < 36) ? 4 : 1; // Sand or Grass
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
            generateChunkBiomeFeatures(chunk, 0, 0.998f, 2, 2, "tree", 1);
            generateChunkBiomeFeatures(chunk, 0, 0.80f, 0, 0, "grass", 1);
            generateChunkBiomeFeatures(chunk, 0, 0.70f, 0, 0, "grassShort", 1);
            generateChunkBiomeFeatures(chunk, 0, 0.67f, 0, 0, "poppy", 1);
            generateChunkBiomeFeatures(chunk, 0, 0.62f, 0, 0, "dandelion", 1);
            break;
        case Chunk::Biome::Forest:
            generateChunkBiomeFeatures(chunk, 0, 0.93f, 2, 2, "tree", 1);
            generateChunkBiomeFeatures(chunk, 0, 0.8f, 0, 0, "grass", 1);
            break;
        case Chunk::Biome::Desert:
            generateChunkBiomeFeatures(chunk, 0, 0.995f, 0, 0, "cactus2", 4);
            generateChunkBiomeFeatures(chunk, 0, 0.98f, 0, 0, "cactus", 4);
            generateChunkBiomeFeatures(chunk, 0, 0.93f, 0, 0, "deadBush", 4);
            break;
    }
}

void generateChunkBiomeFeatures(Chunk& chunk, int margin, float treshold, int xOffset, int zOffset, std::string structureName, int allowedBlockID) {
    ChunkNoises noises = noiseInit();
    const Structure* structure = StructureDB::get(structureName);
    if (!structure) return;

    for (int x = margin; x < Chunk::chunkWidth - margin; ++x) {
        for (int z = margin; z < Chunk::chunkDepth - margin; ++z) {
            float worldX = static_cast<float>(chunk.chunkX * Chunk::chunkWidth + x);
            float worldZ = static_cast<float>(chunk.chunkZ * Chunk::chunkDepth + z);
            float n = noises.featureNoise.GetNoise(worldX, worldZ);
            if (n > treshold) {
                int y = Chunk::chunkHeight - 2;
                while (y > 0 && chunk.blocks[x][y][z].type == 0) --y; {
                    if (chunk.blocks[x][y][z].type == allowedBlockID) {
                        chunk.placeStructure(*structure, x - xOffset, y + 1, z - zOffset);
                    }
                }
            }
        }
    }
}
