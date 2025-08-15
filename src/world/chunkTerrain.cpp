#include <map>
#include "structureDB.hpp"
#include "noise.hpp"
#include "chunkTerrain.hpp"

// Helper function to get biome based on noise value
Chunk::Biome getBiome(float b) {
    if (b >= -1.0f && b < -0.6667f)
        return Chunk::Biome::Forest;
    else if (b >= -0.6667f && b < -0.3333f)
        return Chunk::Biome::Plains;
    else if (b >= -0.3333f && b < 0.0f)
        return Chunk::Biome::FirForest;
    else if (b >= 0.0f && b < 0.3333f)
        return Chunk::Biome::FlowerField;
    else if (b >= 0.3333f && b < 0.6667f)
        return Chunk::Biome::MapleForest;
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
            //(chunk, margin, treshold, xOffset, zOffset, structureName, allowedBlockID, seedOffset, yOffset)
            generateChunkBiomeFeatures(chunk, 0, 0.9997f, 4, 3, "puddle", 1, 0, -4);
            generateChunkBiomeFeatures(chunk, 0, 0.998f, 2, 2, "tree", 1, 1, 0);
            generateChunkBiomeFeatures(chunk, 0, 0.83f, 0, 0, "grass", 1, 2, 0);
            generateChunkBiomeFeatures(chunk, 0, 0.80f, 0, 0, "grassShort", 1, 3, 0);
            generateChunkBiomeFeatures(chunk, 0, 0.96f, 0, 0, "poppy", 1, 4, 0);
            generateChunkBiomeFeatures(chunk, 0, 0.96f, 0, 0, "dandelion", 1, 5, 0);
            break;
        case Chunk::Biome::Forest:
            generateChunkBiomeFeatures(chunk, 0, 0.93f, 2, 2, "tree", 1, 0, 0);
            generateChunkBiomeFeatures(chunk, 0, 0.93f, 0, 0, "grass", 1, 1, 0);
            generateChunkBiomeFeatures(chunk, 0, 0.93f, 0, 0, "grassShort", 1, 2, 0);
            generateChunkBiomeFeatures(chunk, 0, 0.92f, 0, 0, "leavesCarpet", 1, 3, 0);
            break;
        case Chunk::Biome::Desert:
            generateChunkBiomeFeatures(chunk, 0, 0.993f, 0, 0, "cactus2", 4, 0, 0);
            generateChunkBiomeFeatures(chunk, 0, 0.993f, 0, 0, "cactus", 4, 1, 0);
            generateChunkBiomeFeatures(chunk, 0, 0.98f, 0, 0, "deadBush", 4, 2, 0);
            generateChunkBiomeFeatures(chunk, 0, 0.99999f, 8, 8, "pyramid", 4, 3, -1);
            break;
        case Chunk::Biome::FirForest:
            generateChunkBiomeFeatures(chunk, 0, 0.985f, 4, 4, "firTree", 23, 0, 0);
            generateChunkBiomeFeatures(chunk, 0, 0.99f, 4, 4, "smallFirTree", 23, 1, 0);
            generateChunkBiomeFeatures(chunk, 0, 0.80f, 0, 0, "darkGrass", 23, 2, 0);
            generateChunkBiomeFeatures(chunk, 0, 0.88f, 0, 0, "darkGrassShort", 23, 3, 0);
            generateChunkBiomeFeatures(chunk, 0, 0.93f, 0, 0, "pebble", 23, 4, 0);
            break;
        case Chunk::Biome::FlowerField:
            generateChunkBiomeFeatures(chunk, 0, 0.97f, 0, 0, "lavender", 1, 0, 0);
            generateChunkBiomeFeatures(chunk, 0, 0.97f, 0, 0, "crocus", 1, 1, 0);
            generateChunkBiomeFeatures(chunk, 0, 0.97f, 0, 0, "bistort", 1, 2, 0);
            generateChunkBiomeFeatures(chunk, 0, 0.97f, 0, 0, "pinkAnemone", 1, 3, 0);
            generateChunkBiomeFeatures(chunk, 0, 0.97f, 0, 0, "blueSage", 1, 4, 0);
            generateChunkBiomeFeatures(chunk, 0, 0.97f, 0, 0, "poppy", 1, 5, 0);
            generateChunkBiomeFeatures(chunk, 0, 0.97f, 0, 0, "dandelion", 1, 6, 0);
            generateChunkBiomeFeatures(chunk, 0, 0.97f, 0, 0, "grass", 1, 7, 0);
            generateChunkBiomeFeatures(chunk, 0, 0.97f, 0, 0, "grassShort", 1, 8, 0);
            generateChunkBiomeFeatures(chunk, 0, 0.97f, 1, 1, "tinyTree", 1, 9, 0);
            break;
        case Chunk::Biome::MapleForest:
            generateChunkBiomeFeatures(chunk, 0, 0.986f, 2, 2, "redMaple", 1, 0, 0);
            generateChunkBiomeFeatures(chunk, 0, 0.986f, 2, 2, "orangeMaple", 1, 1, 0);
            generateChunkBiomeFeatures(chunk, 0, 0.986f, 2, 2, "yellowMaple", 1, 2, 0);
            generateChunkBiomeFeatures(chunk, 0, 0.90f, 0, 0, "grass", 1, 3, 0);
            generateChunkBiomeFeatures(chunk, 0, 0.90f, 0, 0, "grassShort", 1, 4, 0);
            generateChunkBiomeFeatures(chunk, 0, 0.96f, 0, 0, "poppy", 1, 5, 0);
            generateChunkBiomeFeatures(chunk, 0, 0.96f, 0, 0, "dandelion", 1, 6, 0);
            generateChunkBiomeFeatures(chunk, 0, 0.93f, 0, 0, "leavesCarpet", 1, 7, 0);
            generateChunkBiomeFeatures(chunk, 0, 0.99f, 2, 2, "greenMaple", 1, 8, 0);
            break;
    }
}

void generateChunkBiomeFeatures(Chunk& chunk, int margin, float treshold, int xOffset, int zOffset, std::string structureName, int allowedBlockID, int seedOffset, int yOffset) {
    ChunkNoises noises = noiseInit(seedOffset);
    const Structure* structure = StructureDB::get(structureName);
    if (!structure) return;

    for (int x = margin; x < Chunk::chunkWidth - margin; x++) {
        for (int z = margin; z < Chunk::chunkDepth - margin; z++) {
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

