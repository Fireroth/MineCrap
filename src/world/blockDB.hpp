#pragma once

#include <glm/glm.hpp>
#include <unordered_map>
#include <string>

class BlockDB {
public:
    struct BlockInfo {
        glm::vec2 textureCoords[6];
        bool transparent;
        bool liquid;
        std::string name;
        bool fullBlock;
        std::string modelName;
    };

    static void initialize();
    static const BlockInfo* getBlockInfo(const uint8_t& blockName);

private:
    static std::unordered_map<uint8_t, BlockInfo> blockData;
};
