#include <filesystem>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <nlohmannJSON/json.hpp>
#include "biomeDB.hpp"

std::vector<BiomeData> BiomeDB::biomes;
std::unordered_map<std::string, int> BiomeDB::biomeNameToIndex;

void BiomeDB::init() {
    biomes.clear();
    biomeNameToIndex.clear();

    namespace fs = std::filesystem;
    fs::path biomesDir = fs::current_path() / "biomes";

    if (!fs::exists(biomesDir) || !fs::is_directory(biomesDir)) {
        std::cerr << "BiomeDB::init: could not find 'biomes' directory at " << biomesDir << std::endl;
        return;
    }

    std::vector<fs::path> files;
    for (auto& entry : fs::directory_iterator(biomesDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json")
            files.push_back(entry.path());
    }
    std::sort(files.begin(), files.end());

    for (auto& filePath : files) {
        std::ifstream in(filePath);
        if (!in.is_open()) {
            std::cerr << "BiomeDB::init: failed to open " << filePath << std::endl;
            continue;
        }

        try {
            nlohmann::json j;
            in >> j;

            BiomeData biome;
            biome.name = j.value("name", "Unknown");
            biome.id = j.value("id", "unknown");
            biome.waterBlock = j.value("waterBlock", 9);
            biome.waterLevel = j.value("waterLevel", 37);

            // Parse terrain params
            if (j.contains("terrain") && j["terrain"].is_object()) {
                auto& t = j["terrain"];
                biome.terrain.heightScale = t.value("heightScale", 1.0f);
                biome.terrain.detailWeight = t.value("detailWeight", 0.3f);
                biome.terrain.detail2Weight = t.value("detail2Weight", 0.2f);
                biome.terrain.power = t.value("power", 1.3f);
                biome.terrain.baseHeight = t.value("baseHeight", 30.0f);
                biome.terrain.heightMultiplier = t.value("heightMultiplier", 24.0f);
                biome.terrain.deepenBelowY = t.value("deepenBelowY", 37.0f);
                biome.terrain.deepenFactor = t.value("deepenFactor", 0.5f);
                biome.terrain.flattenAboveY = t.value("flattenAboveY", -1.0f);
            }

            // Parse layers
            if (j.contains("layers") && j["layers"].is_array()) {
                for (auto& layerJson : j["layers"]) {
                    BiomeLayer layer;
                    layer.block = layerJson.value("block", 3);
                    layer.depth = layerJson.value("depth", 1);
                    layer.position = layerJson.value("position", std::string("fill"));
                    layer.aboveY = layerJson.value("aboveY", -1);
                    layer.belowY = layerJson.value("belowY", -1);
                    layer.fallbackBlock = layerJson.value("fallbackBlock", -1);
                    biome.layers.push_back(layer);
                }
            }

            // Parse features
            if (j.contains("features") && j["features"].is_array()) {
                for (auto& featureJson : j["features"]) {
                    BiomeFeature feature;
                    feature.type = featureJson.value("type", std::string("block"));
                    feature.structure = featureJson.value("structure", std::string(""));
                    feature.block = featureJson.value("block", 0);
                    feature.threshold = featureJson.value("threshold", 0.99f);
                    feature.xOffset = featureJson.value("xOffset", 0);
                    feature.zOffset = featureJson.value("zOffset", 0);
                    feature.allowedBlock = featureJson.value("allowedBlock", 1);
                    feature.seedOffset = featureJson.value("seedOffset", 0);
                    feature.yOffset = featureJson.value("yOffset", 0);
                    biome.features.push_back(feature);
                }
            }

            int index = static_cast<int>(biomes.size());
            biomeNameToIndex[biome.id] = index;
            biomes.push_back(biome);

            std::cout << "BiomeDB: loaded biome '" << biome.name << "' (id: " << biome.id << ")" << std::endl;

        } catch (std::exception& e) {
            std::cerr << "BiomeDB::init: JSON parse error in " << filePath << ": " << e.what() << std::endl;
            continue;
        }
    }

    std::cout << "BiomeDB: loaded " << biomes.size() << " biomes" << std::endl;
}

const BiomeData* BiomeDB::getBiome(int index) {
    if (index < 0 || index >= static_cast<int>(biomes.size()))
        return nullptr;
    return &biomes[index];
}

int BiomeDB::getBiomeCount() {
    return static_cast<int>(biomes.size());
}

const BiomeData* BiomeDB::getBiomeByName(const std::string& id) {
    auto it = biomeNameToIndex.find(id);
    if (it != biomeNameToIndex.end())
        return &biomes[it->second];
    return nullptr;
}
