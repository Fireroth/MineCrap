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
