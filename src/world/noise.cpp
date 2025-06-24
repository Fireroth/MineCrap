#include "noise.hpp"
#include "../core/options.hpp"

ChunkNoises noiseInit() {
    ChunkNoises noises;

    int seed = getOptionInt("world_seed", 1234);

    noises.biomeNoise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    noises.biomeNoise.SetCellularReturnType(FastNoiseLite::CellularReturnType_CellValue);
    noises.biomeNoise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_Hybrid);
    noises.biomeNoise.SetFrequency(0.0025f);
    noises.biomeNoise.SetSeed(seed + 10);

    noises.biomeDistortNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noises.biomeDistortNoise.SetFrequency(0.03f);
    noises.biomeDistortNoise.SetSeed(seed + 11);

    noises.baseNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noises.baseNoise.SetFrequency(0.005f);
    noises.baseNoise.SetSeed(seed);

    noises.detailNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noises.detailNoise.SetFrequency(0.02f);
    noises.detailNoise.SetSeed(seed + 1);

    noises.detail2Noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noises.detail2Noise.SetFrequency(0.05f);
    noises.detail2Noise.SetSeed(seed + 2);

    noises.featureNoise.SetNoiseType(FastNoiseLite::NoiseType_Value);
    noises.featureNoise.SetFrequency(500.0f);
    noises.featureNoise.SetSeed(seed + 3);

    return noises;
}
