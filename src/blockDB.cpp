#include "blockDB.hpp"

std::unordered_map<uint8_t, BlockDB::BlockInfo> BlockDB::blockData;

void BlockDB::initialize() {
    blockData[0] = {
        {
            // air
        }
    };

    // Grass
    blockData[1] = {
        {
            glm::vec2(0.0f, 15.0f), // front
            glm::vec2(0.0f, 15.0f), // back
            glm::vec2(0.0f, 15.0f), // left
            glm::vec2(0.0f, 15.0f), // right
            glm::vec2(2.0f, 15.0f), // top
            glm::vec2(1.0f, 15.0f)  // bottom
        }
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
        }
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
        }
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
        }
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
        }
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
        }
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
        }
    };

    //Temporary water implementation
    // Water
    blockData[8] = {
        {
            glm::vec2(0.0f, 13.0f),
            glm::vec2(0.0f, 13.0f),
            glm::vec2(0.0f, 13.0f),
            glm::vec2(0.0f, 13.0f),
            glm::vec2(0.0f, 13.0f),
            glm::vec2(0.0f, 13.0f)
        }
    };

}

const BlockDB::BlockInfo* BlockDB::getBlockInfo(const uint8_t& blockName) {
    auto it = blockData.find(blockName);
    if (it != blockData.end()) {
        return &it->second;
    }
    return nullptr;
}
