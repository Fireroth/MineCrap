#include <fstream>
#include <nlohmannJSON/json.hpp>
#include "structureDB.hpp"

// Chances of block spawning
// 1xxx = 1 in 2
// 2xxx = 1 in5
// 3xxx = 1 in 20

std::unordered_map<std::string, Structure> StructureDB::structures;

const Structure* StructureDB::get(const std::string& name) {
    auto iterator = structures.find(name);
    if (iterator != structures.end())
        return &iterator->second;

    std::string path = "./structures/" + name + ".json";
    std::ifstream file(path);
    if (!file.is_open()) {
        return nullptr;
    }

    nlohmann::json j;
    try {
        file >> j;
    } catch (...) {
        return nullptr;
    }

    // Parse layers
    std::vector<StructureLayer> layers;
    if (j.contains("layers") && j["layers"].is_array()) {
        for (const auto& layer : j["layers"]) {
            StructureLayer l;
            for (const auto& row : layer) {
                std::vector<uint16_t> r;
                for (const auto& cell : row) {
                    r.push_back(cell.get<uint16_t>());
                }
                l.push_back(r);
            }
            layers.push_back(l);
        }
    }

    structures[name] = Structure(name, layers);
    return &structures[name];
}
