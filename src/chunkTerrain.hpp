#pragma once

#include "chunk.hpp"

void generateChunkTerrain(Chunk& chunk);
void generateChunkBiomeFeatures(Chunk& chunk, int margin, float treshold, int xOffset, int zOffset, std::string structureName, int allowedBlockID);
