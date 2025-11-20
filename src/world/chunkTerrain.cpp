#include <map>
#include "structureDB.hpp"
#include "noise.hpp"
#include "chunkTerrain.hpp"

// Helper function to get biome based on noise value
Chunk::Biome getBiome(float b) {
    static const std::vector<Chunk::Biome> biomes = {
        Chunk::Biome::Forest,
        Chunk::Biome::Plains,
        Chunk::Biome::FirForest,
        Chunk::Biome::FlowerField,
        Chunk::Biome::MapleForest,
        Chunk::Biome::Desert
    };

    float normalized = (b + 1.0f) / 2.0f;
    int index = static_cast<int>(normalized * static_cast<float>(biomes.size()));

    if (index >= static_cast<int>(biomes.size()))
        index = static_cast<int>(biomes.size() - 1);

    return biomes[index];
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
        case Chunk::Biome::FirForest:
            heightScale = 0.9f;
            detailWeight = 0.4f;
            power = 1.0f;
            baseHeight = 30.0f;
            break;
        case Chunk::Biome::FlowerField:
            heightScale = 0.7f;
            detailWeight = 0.1f;
            power = 1.0f;
            baseHeight = 29.0f;
            break;
        case Chunk::Biome::MapleForest:
            heightScale = 1.0f;
            detailWeight = 0.4f;
            power = 1.3f;
            baseHeight = 30.0f;
            break;
    }
}

void generateChunkTerrain(Chunk& chunk) {
    const int transitionRadius = 5; // blend over 5 blocks (from each side)
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

    for (int localOffsetX = -transitionRadius; localOffsetX < chunkWidth + transitionRadius; localOffsetX++) {
        for (int localOffsetZ = -transitionRadius; localOffsetZ < chunkDepth + transitionRadius; localOffsetZ++) {
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
            if (height < 37.0f) // Deeper lakes
                height = height - ((37.0f - height) / 2.0f);

            heightCache[localOffsetX + transitionRadius][localOffsetZ + transitionRadius] = height;
        }
    }

    for (int x = 0; x < chunkWidth; x++) {
        for (int z = 0; z < chunkDepth; z++) {
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
            for (int localOffsetX = -transitionRadius; localOffsetX <= transitionRadius && !hasDifferentBiome; localOffsetX++) {
                for (int localOffsetZ = -transitionRadius; localOffsetZ <= transitionRadius && !hasDifferentBiome; localOffsetZ++) {
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

                for (int localOffsetX = -transitionRadius; localOffsetX <= transitionRadius; localOffsetX++) {
                    for (int localOffsetZ = -transitionRadius; localOffsetZ <= transitionRadius; localOffsetZ++) {
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

            for (int y = 0; y < chunkHeight; y++) {
                if (y == 0) {
                    chunk.blocks[x][y][z].type = 6; // Bedrock
                } else if (y > height) {
                    chunk.blocks[x][y][z].type = (y < 37) ? 9 : 0; // Water or air
                    continue;
                } else if (y == height) {
                    switch (finalBiome) {
                        case Chunk::Biome::Plains:
                        case Chunk::Biome::Forest:
                        case Chunk::Biome::FlowerField:
                        case Chunk::Biome::MapleForest:
                            // If grass would be generated below y 36, use sand instead
                            chunk.blocks[x][y][z].type = (y < 36) ? 4 : 1; // Sand or Grass Block
                            break;
                        case Chunk::Biome::FirForest:
                            chunk.blocks[x][y][z].type = (y < 36) ? 4 : 23; // Sand or Dark Grass Block
                            break;
                        case Chunk::Biome::Desert:
                            chunk.blocks[x][y][z].type = 4; // Sand
                            break;
                    }
                } else if (y >= height - 3) {
                    switch (finalBiome) {
                        case Chunk::Biome::Plains:
                        case Chunk::Biome::Forest:
                        case Chunk::Biome::FirForest:
                        case Chunk::Biome::FlowerField:
                        case Chunk::Biome::MapleForest:
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
                        case Chunk::Biome::FirForest:
                        case Chunk::Biome::FlowerField:
                        case Chunk::Biome::MapleForest:
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
            //(chunk, treshold, xOffset, zOffset, structureName/blockID, allowedBlockID, seedOffset, yOffset)
            generateChunkBiomeFeatures(chunk, 0.9997f, 4, 3, "puddle", 1, 0, -4);
            generateChunkBiomeFeatures(chunk, 0.998f, 2, 2, "tree", 1, 1, 0);
            generateChunkBiomeBlocks(chunk, 0.83f, 0, 0, 17, 1, 2, 0); // Grass
            generateChunkBiomeBlocks(chunk, 0.80f, 0, 0, 18, 1, 3, 0); // Grass Short
            generateChunkBiomeBlocks(chunk, 0.96f, 0, 0, 20, 1, 4, 0); // Poppy
            generateChunkBiomeBlocks(chunk, 0.96f, 0, 0, 21, 1, 5, 0); // Dandelion
            break;
        case Chunk::Biome::Forest:
            generateChunkBiomeFeatures(chunk, 0.93f, 2, 2, "tree", 1, 0, 0);
            generateChunkBiomeBlocks(chunk, 0.93f, 0, 0, 17, 1, 1, 0); // Grass
            generateChunkBiomeBlocks(chunk, 0.93f, 0, 0, 18, 1, 2, 0); // Grass Short
            generateChunkBiomeBlocks(chunk, 0.92f, 0, 0, 41, 1, 3, 0); // Leaves Carpet
            generateChunkBiomeBlocks(chunk, 0.997f, 0, 0, 60, 1, 4, 0); // Brown Mushroom
            generateChunkBiomeBlocks(chunk, 0.997f, 0, 0, 61, 1, 5, 0); // Red Mushroom
            break;
        case Chunk::Biome::Desert:
            generateChunkBiomeFeatures(chunk, 0.997f, 0, 0, "cactus2", 4, 0, 0);
            generateChunkBiomeFeatures(chunk, 0.997f, 0, 0, "cactus", 4, 1, 0);
            generateChunkBiomeBlocks(chunk, 0.985f, 0, 0, 19, 4, 2, 0); // Dead Bush
            generateChunkBiomeFeatures(chunk, 0.99999f, 8, 8, "pyramid", 4, 3, -1);
            generateChunkBiomeFeatures(chunk, 0.998f, 0, 0, "flowerCactus", 4, 4, 0);
            generateChunkBiomeBlocks(chunk, 0.985f, 0, 0, 59, 4, 5, 0); // Dead Grass
            break;
        case Chunk::Biome::FirForest:
            generateChunkBiomeFeatures(chunk, 0.985f, 4, 4, "firTree", 23, 0, 0);
            generateChunkBiomeFeatures(chunk, 0.99f, 4, 4, "smallFirTree", 23, 1, 0);
            generateChunkBiomeBlocks(chunk, 0.80f, 0, 0, 24, 23, 2, 0); // Dark Grass
            generateChunkBiomeBlocks(chunk, 0.88f, 0, 0, 26, 23, 3, 0); // Dark Grass Short
            generateChunkBiomeBlocks(chunk, 0.93f, 0, 0, 25, 23, 4, 0); // Pebble
            generateChunkBiomeBlocks(chunk, 0.98f, 0, 0, 56, 9, 5, 0); // Lily Pad
            generateChunkBiomeBlocks(chunk, 0.98f, 0, 0, 60, 23, 6, 0); // Brown Mushroom
            generateChunkBiomeBlocks(chunk, 0.98f, 0, 0, 61, 23, 7, 0); // Red Mushroom
            generateChunkBiomeFeatures(chunk, 0.998f, 2, 2, "rock", 23, 8, -1);
            break;
        case Chunk::Biome::FlowerField:
            generateChunkBiomeBlocks(chunk, 0.97f, 0, 0, 34, 1, 0, 0); // Lavender
            generateChunkBiomeBlocks(chunk, 0.97f, 0, 0, 33, 1, 1, 0); // Crocus
            generateChunkBiomeBlocks(chunk, 0.97f, 0, 0, 30, 1, 2, 0); // Bistort
            generateChunkBiomeBlocks(chunk, 0.97f, 0, 0, 29, 1, 3, 0); // Pink Anemone
            generateChunkBiomeBlocks(chunk, 0.97f, 0, 0, 28, 1, 4, 0); // Blue Sage
            generateChunkBiomeBlocks(chunk, 0.97f, 0, 0, 20, 1, 5, 0); // Poppy
            generateChunkBiomeBlocks(chunk, 0.97f, 0, 0, 21, 1, 6, 0); // Dandelion
            generateChunkBiomeBlocks(chunk, 0.97f, 0, 0, 17, 1, 7, 0); // Grass
            generateChunkBiomeBlocks(chunk, 0.97f, 0, 0, 18, 1, 8, 0); // Grass Short
            generateChunkBiomeFeatures(chunk, 0.97f, 1, 1, "tinyTree", 1, 9, 0);
            generateChunkBiomeBlocks(chunk, 0.97f, 0, 0, 55, 1, 10, 0); // Red Rose
            break;
        case Chunk::Biome::MapleForest:
            generateChunkBiomeFeatures(chunk, 0.986f, 2, 2, "redMaple", 1, 0, 0);
            generateChunkBiomeFeatures(chunk, 0.986f, 2, 2, "orangeMaple", 1, 1, 0);
            generateChunkBiomeFeatures(chunk, 0.986f, 2, 2, "yellowMaple", 1, 2, 0);
            generateChunkBiomeBlocks(chunk, 0.90f, 0, 0, 17, 1, 3, 0); // Grass
            generateChunkBiomeBlocks(chunk, 0.90f, 0, 0, 18, 1, 4, 0); // Grass Short
            generateChunkBiomeBlocks(chunk, 0.96f, 0, 0, 20, 1, 5, 0); // Poppy
            generateChunkBiomeBlocks(chunk, 0.96f, 0, 0, 21, 1, 6, 0); // Dandelion
            generateChunkBiomeBlocks(chunk, 0.93f, 0, 0, 52, 1, 7, 0); // Maple Leaves Carpet
            generateChunkBiomeFeatures(chunk, 0.99f, 2, 2, "greenMaple", 1, 8, 0);
            break;
    }
}

void generateChunkBiomeFeatures(Chunk& chunk, float treshold, int xOffset, int zOffset, std::string structureName, int allowedBlockID, int seedOffset, int yOffset) {
    ChunkNoises noises = noiseInit(seedOffset);
    const Structure* structure = StructureDB::get(structureName);
    if (!structure) return;

    for (int x = 0; x < Chunk::chunkWidth; x++) {
        for (int z = 0; z < Chunk::chunkDepth; z++) {
            float worldX = static_cast<float>(chunk.chunkX * Chunk::chunkWidth + x);
            float worldZ = static_cast<float>(chunk.chunkZ * Chunk::chunkDepth + z);
            float n = noises.featureNoise.GetNoise(worldX, worldZ);
            if (n > treshold) {
                int y = Chunk::chunkHeight - 2;
                while (y > 0 && chunk.blocks[x][y][z].type == 0) y--; {
                    if (chunk.blocks[x][y][z].type == allowedBlockID) {
                        chunk.placeStructure(*structure, x - xOffset, (y + 1) + yOffset, z - zOffset);
                    }
                }
            }
        }
    }
}

void generateChunkBiomeBlocks(Chunk& chunk, float treshold, int xOffset, int zOffset, int blockID, int allowedBlockID, int seedOffset, int yOffset) {
    ChunkNoises noises = noiseInit(seedOffset);

    for (int x = 0; x < Chunk::chunkWidth; x++) {
        for (int z = 0; z < Chunk::chunkDepth; z++) {
            float worldX = static_cast<float>(chunk.chunkX * Chunk::chunkWidth + x);
            float worldZ = static_cast<float>(chunk.chunkZ * Chunk::chunkDepth + z);
            float n = noises.featureNoise.GetNoise(worldX, worldZ);
            if (n > treshold) {
                int y = Chunk::chunkHeight - 2;
                while (y > 0 && chunk.blocks[x][y][z].type == 0) y--; {
                    if (chunk.blocks[x][y][z].type == allowedBlockID) {
                        int tx = x - xOffset;
                        int tz = z - zOffset;
                        int ty = (y + 1) + yOffset;

                        if (tx >= 0 && tx < Chunk::chunkWidth && tz >= 0 && tz < Chunk::chunkDepth && ty >= 0 && ty < Chunk::chunkHeight) {
                            chunk.blocks[tx][ty][tz].type = static_cast<uint8_t>(blockID);
                        }
                    }
                }
            }
        }
    }
}
