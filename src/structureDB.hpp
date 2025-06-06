#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <unordered_map>

using StructureLayer = std::vector<std::vector<uint8_t>>;

class Structure {
public:
    std::string name;
    std::vector<StructureLayer> layers;

    Structure() = default;
    Structure(const std::string& name, const std::vector<StructureLayer>& layers)
        : name(name), layers(layers) {}
};

class StructureDB {
public:
    static void initialize();
    static const Structure* get(const std::string& name);

private:
    static std::unordered_map<std::string, Structure> structures;
};
