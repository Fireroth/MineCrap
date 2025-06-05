#include "blockDB.hpp"

std::unordered_map<uint8_t, BlockDB::BlockInfo> BlockDB::blockData;

void BlockDB::initialize() {
    // Grass
    blockData[1] = {
        {
            glm::vec2(0.0f, 15.0f), // front
            glm::vec2(0.0f, 15.0f), // back
            glm::vec2(0.0f, 15.0f), // left
            glm::vec2(0.0f, 15.0f), // right
            glm::vec2(2.0f, 15.0f), // top
            glm::vec2(1.0f, 15.0f)  // bottom
        },
        false // transparency
    };

    // Dirt
    blockData[2] = {
        {
            glm::vec2(1.0f, 15.0f),
            glm::vec2(1.0f, 15.0f),
            glm::vec2(1.0f, 15.0f),
            glm::vec2(1.0f, 15.0f),
            glm::vec2(1.0f, 15.0f),
            glm::vec2(1.0f, 15.0f)
        },
        false
    };

    // Stone
    blockData[3] = {
        {
            glm::vec2(3.0f, 15.0f),
            glm::vec2(3.0f, 15.0f),
            glm::vec2(3.0f, 15.0f),
            glm::vec2(3.0f, 15.0f),
            glm::vec2(3.0f, 15.0f),
            glm::vec2(3.0f, 15.0f)
        },
        false
    };

    // Sand
    blockData[4] = {
        {
            glm::vec2(4.0f, 15.0f),
            glm::vec2(4.0f, 15.0f),
            glm::vec2(4.0f, 15.0f),
            glm::vec2(4.0f, 15.0f),
            glm::vec2(4.0f, 15.0f),
            glm::vec2(4.0f, 15.0f)
        },
        false
    };

    // Log
    blockData[5] = {
        {
            glm::vec2(2.0f, 14.0f),
            glm::vec2(2.0f, 14.0f),
            glm::vec2(2.0f, 14.0f),
            glm::vec2(2.0f, 14.0f),
            glm::vec2(3.0f, 14.0f),
            glm::vec2(3.0f, 14.0f)
        },
        false
    };

    // Bedrock
    blockData[6] = {
        {
            glm::vec2(1.0f, 14.0f),
            glm::vec2(1.0f, 14.0f),
            glm::vec2(1.0f, 14.0f),
            glm::vec2(1.0f, 14.0f),
            glm::vec2(1.0f, 14.0f),
            glm::vec2(1.0f, 14.0f)
        },
        false
    };

    //Gravel
    blockData[7] = {
        {
            glm::vec2(5.0f, 15.0f),
            glm::vec2(5.0f, 15.0f),
            glm::vec2(5.0f, 15.0f),
            glm::vec2(5.0f, 15.0f),
            glm::vec2(5.0f, 15.0f),
            glm::vec2(5.0f, 15.0f)
        },
        false
    };

    //Bricks
    blockData[8] = {
        {
            glm::vec2(4.0f, 14.0f),
            glm::vec2(4.0f, 14.0f),
            glm::vec2(4.0f, 14.0f),
            glm::vec2(4.0f, 14.0f),
            glm::vec2(4.0f, 14.0f),
            glm::vec2(4.0f, 14.0f)
        },
        false
    };

    //Temporary water and lava implementation
    // Water
    blockData[9] = {
        {
            glm::vec2(0.0f, 13.0f),
            glm::vec2(0.0f, 13.0f),
            glm::vec2(0.0f, 13.0f),
            glm::vec2(0.0f, 13.0f),
            glm::vec2(0.0f, 13.0f),
            glm::vec2(0.0f, 13.0f)
        },
        true
    };

    // Lava
    blockData[10] = {
        {
            glm::vec2(1.0f, 13.0f),
            glm::vec2(1.0f, 13.0f),
            glm::vec2(1.0f, 13.0f),
            glm::vec2(1.0f, 13.0f),
            glm::vec2(1.0f, 13.0f),
            glm::vec2(1.0f, 13.0f)
        },
        true
    };

    //Leaves
    blockData[11] = {
        {
            glm::vec2(6.0f, 15.0f),
            glm::vec2(6.0f, 15.0f),
            glm::vec2(6.0f, 15.0f),
            glm::vec2(6.0f, 15.0f),
            glm::vec2(6.0f, 15.0f),
            glm::vec2(6.0f, 15.0f)
        },
        true
    };
    
    //Cactus
    blockData[12] = {
        {
            glm::vec2(7.0f, 15.0f),
            glm::vec2(7.0f, 15.0f),
            glm::vec2(7.0f, 15.0f),
            glm::vec2(7.0f, 15.0f),
            glm::vec2(7.0f, 14.0f),
            glm::vec2(8.0f, 15.0f)
        },
        false
    };
}

const BlockDB::BlockInfo* BlockDB::getBlockInfo(const uint8_t& blockName) {
    auto it = blockData.find(blockName);
    if (it != blockData.end()) {
        return &it->second;
    }
    return nullptr;
}
