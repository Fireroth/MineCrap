#include "structureDB.hpp"

std::unordered_map<std::string, Structure> StructureDB::structures;

void StructureDB::initialize() {
    std::vector<StructureLayer> treeLayers = {
        {
            {0,0,0,0,0},
            {0,0,0,0,0},
            {0,0,5,0,0},
            {0,0,0,0,0},
            {0,0,0,0,0}
        },
        {
            {0,0,0,0,0},
            {0,0,0,0,0},
            {0,0,5,0,0},
            {0,0,0,0,0},
            {0,0,0,0,0}
        },
        {
            {0,0,0,0,0},
            {0,0,0,0,0},
            {0,0,5,0,0},
            {0,0,0,0,0},
            {0,0,0,0,0}
        },
        {
            {0,11,11,11,0},
            {11,11,11,11,11},
            {11,11,5,11,11},
            {11,11,11,11,11},
            {0,11,11,11,0}
        },
        {
            {0,11,11,11,0},
            {11,11,11,11,11},
            {11,11,5,11,11},
            {11,11,11,11,11},
            {0,11,11,11,0}
        },
        {
            {0,0,0,0,0},
            {0,11,11,11,0},
            {0,11,11,11,0},
            {0,11,11,11,0},
            {0,0,0,0,0}
        },
        {
            {0,0,0,0,0},
            {0,0,11,0,0},
            {0,11,11,11,0},
            {0,0,11,0,0},
            {0,0,0,0,0}
        }
    };
    structures["tree"] = Structure("tree", treeLayers);

    std::vector<StructureLayer> cactusLayers = {
        { {12} },
        { {12} },
        { {12} }
    };
    structures["cactus"] = Structure("cactus", cactusLayers);

    std::vector<StructureLayer> cactus2Layers = {
        { {12} },
        { {12} },
        { {12} },
        { {12} }
    };
    structures["cactus2"] = Structure("cactus2", cactus2Layers);
}

const Structure* StructureDB::get(const std::string& name) {
    auto iterator = structures.find(name);
    if (iterator != structures.end())
        return &iterator->second;
    return nullptr;
}
