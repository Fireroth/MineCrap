#pragma once

#include <glm/glm.hpp>
#include <unordered_map>
#include <array>
#include <string>
#include <vector>

class BlockDB {
public:
    struct BlockInfo {
        glm::vec2 textureCoords[6];
        std::vector<std::array<glm::vec2,6>> multiTextureCoords;
        bool transparent;
        bool liquid;
        std::string name;
        std::string modelName;
        bool renderFacesInBetween;
        std::string tabName;
    };

    static void init();
    static const BlockInfo* getBlockInfo(const uint8_t& blockName);

private:
    static std::unordered_map<uint8_t, BlockInfo> blockData;
};
