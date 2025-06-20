#pragma once

#include <FastNoiseLite.h>

struct ChunkNoises {
    FastNoiseLite biomeNoise;
    FastNoiseLite baseNoise;
    FastNoiseLite detailNoise;
    FastNoiseLite detail2Noise;
    FastNoiseLite featureNoise;
};

ChunkNoises noiseInit();
