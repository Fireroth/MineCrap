#include "noise.hpp"
#include "../core/options.hpp"

ChunkNoises noiseInit() {
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

    noises.featureNoise.SetNoiseType(FastNoiseLite::NoiseType_Value);
    noises.featureNoise.SetFrequency(200.0f);
    noises.featureNoise.SetSeed(seed + 3000);

    return noises;
}
