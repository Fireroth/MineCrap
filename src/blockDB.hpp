#pragma once

#include <glm/glm.hpp>
#include <unordered_map>
#include <string>

class BlockDB {
public:
    struct BlockInfo {
        glm::vec2 textureCoords[6];
    };

    static void initialize();
    static const BlockInfo* getBlockInfo(const std::string& blockName);

private:
    static std::unordered_map<std::string, BlockInfo> blockData;
};
