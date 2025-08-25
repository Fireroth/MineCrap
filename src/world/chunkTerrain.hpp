#pragma once

#include "chunk.hpp"

void generateChunkTerrain(Chunk& chunk);
void generateChunkBiomeFeatures(Chunk& chunk, float treshold, int xOffset, int zOffset,
                                std::string structureName, int allowedBlockID, int seedOffset, int yOffset);
