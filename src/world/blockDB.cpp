#include "blockDB.hpp"
#include "../core/options.hpp"

static int fasterTrees = getOptionInt("faster_trees", 0);

std::unordered_map<uint8_t, BlockDB::BlockInfo> BlockDB::blockData;

void BlockDB::initialize() {
    blockData[1] = {
        {
            glm::vec2(0.0f, 15.0f), // front
            glm::vec2(0.0f, 15.0f), // back
            glm::vec2(0.0f, 15.0f), // left
            glm::vec2(0.0f, 15.0f), // right
            glm::vec2(2.0f, 15.0f), // top
            glm::vec2(1.0f, 15.0f)  // bottom
        },
        false, // Transparent
        false, // Liquid
        "Grass block", // Name
        true, // Full block
        "cube"   // Model
    };

    blockData[2] = {
        {
            glm::vec2(1.0f, 15.0f),
            glm::vec2(1.0f, 15.0f),
            glm::vec2(1.0f, 15.0f),
            glm::vec2(1.0f, 15.0f),
            glm::vec2(1.0f, 15.0f),
            glm::vec2(1.0f, 15.0f)
        },
        false,
        false,
        "Dirt",
        true,
        "cube"
    };

    blockData[3] = {
        {
            glm::vec2(3.0f, 15.0f),
            glm::vec2(3.0f, 15.0f),
            glm::vec2(3.0f, 15.0f),
            glm::vec2(3.0f, 15.0f),
            glm::vec2(3.0f, 15.0f),
            glm::vec2(3.0f, 15.0f)
        },
        false,
        false,
        "Stone",
        true,
        "cube"
    };

    blockData[4] = {
        {
            glm::vec2(4.0f, 15.0f),
            glm::vec2(4.0f, 15.0f),
            glm::vec2(4.0f, 15.0f),
            glm::vec2(4.0f, 15.0f),
            glm::vec2(4.0f, 15.0f),
            glm::vec2(4.0f, 15.0f)
        },
        false,
        false,
        "Sand",
        true,
        "cube"
    };

    blockData[5] = {
        {
            glm::vec2(2.0f, 14.0f),
            glm::vec2(2.0f, 14.0f),
            glm::vec2(2.0f, 14.0f),
            glm::vec2(2.0f, 14.0f),
            glm::vec2(3.0f, 14.0f),
            glm::vec2(3.0f, 14.0f)
        },
        false,
        false,
        "Oak Log",
        true,
        "cube"
    };

    blockData[6] = {
        {
            glm::vec2(1.0f, 14.0f),
            glm::vec2(1.0f, 14.0f),
            glm::vec2(1.0f, 14.0f),
            glm::vec2(1.0f, 14.0f),
            glm::vec2(1.0f, 14.0f),
            glm::vec2(1.0f, 14.0f)
        },
        false,
        false,
        "Bedrock",
        true,
        "cube"
    };

    blockData[7] = {
        {
            glm::vec2(5.0f, 15.0f),
            glm::vec2(5.0f, 15.0f),
            glm::vec2(5.0f, 15.0f),
            glm::vec2(5.0f, 15.0f),
            glm::vec2(5.0f, 15.0f),
            glm::vec2(5.0f, 15.0f)
        },
        false,
        false,
        "Gravel",
        true,
        "cube"
    };

    blockData[8] = {
        {
            glm::vec2(4.0f, 14.0f),
            glm::vec2(4.0f, 14.0f),
            glm::vec2(4.0f, 14.0f),
            glm::vec2(4.0f, 14.0f),
            glm::vec2(4.0f, 14.0f),
            glm::vec2(4.0f, 14.0f)
        },
        false,
        false,
        "Bricks",
        true,
        "cube"
    };

    //Temporary water and lava implementation
    blockData[9] = {
        {
            glm::vec2(0.0f, 13.0f),
            glm::vec2(0.0f, 13.0f),
            glm::vec2(0.0f, 13.0f),
            glm::vec2(0.0f, 13.0f),
            glm::vec2(0.0f, 13.0f),
            glm::vec2(0.0f, 13.0f)
        },
        true,
        true,
        "Water",
        true,
        "cube"
    };

    blockData[10] = {
        {
            glm::vec2(1.0f, 13.0f),
            glm::vec2(1.0f, 13.0f),
            glm::vec2(1.0f, 13.0f),
            glm::vec2(1.0f, 13.0f),
            glm::vec2(1.0f, 13.0f),
            glm::vec2(1.0f, 13.0f)
        },
        true,
        true,
        "Lava",
        true,
        "cube"
    };

    if (fasterTrees) {
        blockData[11] = {
        {
            glm::vec2(6.0f, 15.0f),
            glm::vec2(6.0f, 15.0f),
            glm::vec2(6.0f, 15.0f),
            glm::vec2(6.0f, 15.0f),
            glm::vec2(6.0f, 15.0f),
            glm::vec2(6.0f, 15.0f)
        },
        false,
        false,
        "Leaves",
        true,
        "cube"
        };

    } else {
        blockData[11] = {
        {
            glm::vec2(5.0f, 14.0f),
            glm::vec2(5.0f, 14.0f),
            glm::vec2(5.0f, 14.0f),
            glm::vec2(5.0f, 14.0f),
            glm::vec2(5.0f, 14.0f),
            glm::vec2(5.0f, 14.0f)
        },
        true,
        false,
        "Leaves",
        true,
        "cube"
        };
    }

    blockData[12] = {
        {
            glm::vec2(7.0f, 15.0f),
            glm::vec2(7.0f, 15.0f),
            glm::vec2(7.0f, 15.0f),
            glm::vec2(7.0f, 15.0f),
            glm::vec2(7.0f, 14.0f),
            glm::vec2(8.0f, 15.0f)
        },
        false,
        false,
        "Cactus",
        false,
        "cactus"
    };

    blockData[13] = {
        {
            glm::vec2(3.0f, 15.0f),
            glm::vec2(3.0f, 15.0f),
            glm::vec2(3.0f, 15.0f),
            glm::vec2(3.0f, 15.0f),
            glm::vec2(3.0f, 15.0f),
            glm::vec2(3.0f, 15.0f)
        },
        false,
        false,
        "Stone slab",
        false,
        "slab"
    };

    blockData[14] = {
        {
            glm::vec2(6.0f, 14.0f),
            glm::vec2(6.0f, 14.0f),
            glm::vec2(6.0f, 14.0f),
            glm::vec2(6.0f, 14.0f),
            glm::vec2(6.0f, 14.0f),
            glm::vec2(6.0f, 14.0f)
        },
        false,
        false,
        "Oak Plank",
        true,
        "cube"
    };

    blockData[15] = {
        {
            glm::vec2(6.0f, 14.0f),
            glm::vec2(6.0f, 14.0f),
            glm::vec2(6.0f, 14.0f),
            glm::vec2(6.0f, 14.0f),
            glm::vec2(6.0f, 14.0f),
            glm::vec2(6.0f, 14.0f)
        },
        false,
        false,
        "Oak Plank Slab",
        false,
        "slab"
    };

    blockData[16] = {
        {
            glm::vec2(8.0f, 14.0f),
            glm::vec2(8.0f, 14.0f),
            glm::vec2(8.0f, 14.0f),
            glm::vec2(8.0f, 14.0f),
            glm::vec2(8.0f, 14.0f),
            glm::vec2(8.0f, 14.0f)
        },
        true,
        false,
        "Glass",
        true,
        "cube"
    };

    blockData[17] = {
        {
            glm::vec2(2.0f, 13.0f),
            glm::vec2(2.0f, 13.0f),
            glm::vec2(2.0f, 13.0f),
            glm::vec2(2.0f, 13.0f)
        },
        true,
        false,
        "Medium Grass",
        true,
        "cross"
    };

    blockData[18] = {
        {
            glm::vec2(3.0f, 13.0f),
            glm::vec2(3.0f, 13.0f),
            glm::vec2(3.0f, 13.0f),
            glm::vec2(3.0f, 13.0f)
        },
        true,
        false,
        "Short Grass",
        true,
        "cross"
    };

    blockData[19] = {
        {
            glm::vec2(4.0f, 13.0f),
            glm::vec2(4.0f, 13.0f),
            glm::vec2(4.0f, 13.0f),
            glm::vec2(4.0f, 13.0f)
        },
        true,
        false,
        "Dead Bush",
        true,
        "cross"
    };

    blockData[20] = {
        {
            glm::vec2(5.0f, 13.0f),
            glm::vec2(5.0f, 13.0f),
            glm::vec2(5.0f, 13.0f),
            glm::vec2(5.0f, 13.0f)
        },
        true,
        false,
        "Poppy",
        true,
        "cross"
    };

    blockData[21] = {
        {
            glm::vec2(6.0f, 13.0f),
            glm::vec2(6.0f, 13.0f),
            glm::vec2(6.0f, 13.0f),
            glm::vec2(6.0f, 13.0f)
        },
        true,
        false,
        "Dandelion",
        true,
        "cross"
    };
}

const BlockDB::BlockInfo* BlockDB::getBlockInfo(const uint8_t& blockName) {
    auto iterator = blockData.find(blockName);
    if (iterator != blockData.end()) {
        return &iterator->second;
    }
    return nullptr;
}
