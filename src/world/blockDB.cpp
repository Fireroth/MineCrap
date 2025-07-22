#include "blockDB.hpp"
#include "../core/options.hpp"

static int fasterTrees = getOptionInt("faster_trees", 0);

std::unordered_map<uint8_t, BlockDB::BlockInfo> BlockDB::blockData;

void BlockDB::init() {
    blockData[1] = {
        {
            glm::vec2(0.0f, 15.0f), // front
            glm::vec2(0.0f, 15.0f), // back
            glm::vec2(0.0f, 15.0f), // left
            glm::vec2(0.0f, 15.0f), // right
            glm::vec2(2.0f, 15.0f), // top
            glm::vec2(1.0f, 15.0f)  // bottom
        },
        false,          // Transparent
        false,          // Liquid
        "Grass block",  // Name
        "cube",         // Model
        false           // Render faces in between
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
        "cube",
        false
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
        "cube",
        false
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
        "cube",
        false
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
        "cube",
        false
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
        "cube",
        false
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
        "cube",
        false
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
        "cube",
        false
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
        "liquid",
        false
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
        false,
        "Lava",
        "liquid",
        false
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
        "cube",
        false
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
        "cube",
        true
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
        "cactus",
        false
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
        "slab",
        false
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
        "cube",
        false
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
        "slab",
        false
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
        "cube",
        false
    };

    blockData[17] = {
        {
            glm::vec2(2.0f, 13.0f),
            glm::vec2(2.0f, 13.0f)
        },
        true,
        false,
        "Medium Grass",
        "cross",
        false
    };

    blockData[18] = {
        {
            glm::vec2(3.0f, 13.0f),
            glm::vec2(3.0f, 13.0f)
        },
        true,
        false,
        "Short Grass",
        "cross",
        false
    };

    blockData[19] = {
        {
            glm::vec2(4.0f, 13.0f),
            glm::vec2(4.0f, 13.0f)
        },
        true,
        false,
        "Dead Bush",
        "cross",
        false
    };

    blockData[20] = {
        {
            glm::vec2(5.0f, 13.0f),
            glm::vec2(5.0f, 13.0f)
        },
        true,
        false,
        "Poppy",
        "cross",
        false
    };

    blockData[21] = {
        {
            glm::vec2(6.0f, 13.0f),
            glm::vec2(6.0f, 13.0f)
        },
        true,
        false,
        "Dandelion",
        "cross",
        false
    };

    if (fasterTrees) {
        blockData[22] = {
        {
            glm::vec2(8.0f, 13.0f),
            glm::vec2(8.0f, 13.0f),
            glm::vec2(8.0f, 13.0f),
            glm::vec2(8.0f, 13.0f),
            glm::vec2(8.0f, 13.0f),
            glm::vec2(8.0f, 13.0f)
        },
        false,
        false,
        "Fir Leaves",
        "cube",
        true
        };

    } else {
        blockData[22] = {
        {
            glm::vec2(7.0f, 13.0f),
            glm::vec2(7.0f, 13.0f),
            glm::vec2(7.0f, 13.0f),
            glm::vec2(7.0f, 13.0f),
            glm::vec2(7.0f, 13.0f),
            glm::vec2(7.0f, 13.0f)
        },
        true,
        false,
        "Fir Leaves",
        "cube",
        true
        };
    };

    blockData[23] = {
        {
            glm::vec2(1.0f, 12.0f),
            glm::vec2(1.0f, 12.0f),
            glm::vec2(1.0f, 12.0f),
            glm::vec2(1.0f, 12.0f),
            glm::vec2(2.0f, 12.0f),
            glm::vec2(1.0f, 15.0f)
        },
        false,
        false,
        "Dark Grass block",
        "cube",
        false
    };

    blockData[24] = {
        {
            glm::vec2(3.0f, 12.0f),
            glm::vec2(3.0f, 12.0f)
        },
        true,
        false,
        "Dark Medium Grass",
        "cross",
        false
    };

    blockData[25] = {
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
        "Pebble",
        "pebble",
        false
    };

    blockData[26] = {
        {
            glm::vec2(4.0f, 12.0f),
            glm::vec2(4.0f, 12.0f)
        },
        true,
        false,
        "Dark Short Grass",
        "cross",
        false
    };

    blockData[27] = {
        {
            glm::vec2(5.0f, 12.0f),
            glm::vec2(5.0f, 12.0f),
            glm::vec2(5.0f, 12.0f),
            glm::vec2(5.0f, 12.0f),
            glm::vec2(6.0f, 12.0f),
            glm::vec2(6.0f, 12.0f)
        },
        false,
        false,
        "Fir Log",
        "cube",
        false
    };

    blockData[28] = {
        {
            glm::vec2(7.0f, 12.0f),
            glm::vec2(7.0f, 12.0f)
        },
        true,
        false,
        "Blue Sage",
        "cross",
        false
    };

    blockData[29] = {
        {
            glm::vec2(8.0f, 12.0f),
            glm::vec2(8.0f, 12.0f)
        },
        true,
        false,
        "Pink Anemone",
        "cross",
        false
    };

    blockData[30] = {
        {
            glm::vec2(7.0f, 11.0f),
            glm::vec2(7.0f, 11.0f)
        },
        true,
        false,
        "Bistort",
        "cross",
        false
    };

    blockData[31] = {
        {
            glm::vec2(6.0f, 11.0f),
            glm::vec2(6.0f, 11.0f),
            glm::vec2(6.0f, 11.0f),
            glm::vec2(6.0f, 11.0f),
            glm::vec2(6.0f, 11.0f),
            glm::vec2(6.0f, 11.0f)
        },
        false,
        false,
        "Fir Plank",
        "cube",
        false
    };

    blockData[32] = {
        {
            glm::vec2(6.0f, 11.0f),
            glm::vec2(6.0f, 11.0f),
            glm::vec2(6.0f, 11.0f),
            glm::vec2(6.0f, 11.0f),
            glm::vec2(6.0f, 11.0f),
            glm::vec2(6.0f, 11.0f)
        },
        false,
        false,
        "Fir Plank Slab",
        "slab",
        false
    };

    blockData[33] = {
        {
            glm::vec2(8.0f, 11.0f),
            glm::vec2(8.0f, 11.0f)
        },
        true,
        false,
        "Crocus",
        "cross",
        false
    };

    blockData[34] = {
        {
            glm::vec2(9.0f, 12.0f),
            glm::vec2(9.0f, 12.0f)
        },
        true,
        false,
        "Lavender",
        "cross",
        false
    };

    if (fasterTrees) {
        blockData[35] = {
            {
                glm::vec2(3.0f, 10.0f),
                glm::vec2(3.0f, 10.0f),
                glm::vec2(3.0f, 10.0f),
                glm::vec2(3.0f, 10.0f),
                glm::vec2(3.0f, 10.0f),
                glm::vec2(3.0f, 10.0f)
            },
            false,
            false,
            "Red Maple Leaves",
            "cube",
            false
        };

        blockData[36] = {
            {
                glm::vec2(4.0f, 10.0f),
                glm::vec2(4.0f, 10.0f),
                glm::vec2(4.0f, 10.0f),
                glm::vec2(4.0f, 10.0f),
                glm::vec2(4.0f, 10.0f),
                glm::vec2(4.0f, 10.0f)
            },
            false,
            false,
            "Orange Maple Leaves",
            "cube",
            false
        };

        blockData[37] = {
            {
                glm::vec2(5.0f, 10.0f),
                glm::vec2(5.0f, 10.0f),
                glm::vec2(5.0f, 10.0f),
                glm::vec2(5.0f, 10.0f),
                glm::vec2(5.0f, 10.0f),
                glm::vec2(5.0f, 10.0f)
            },
            false,
            false,
            "Yellow Maple Leaves",
            "cube",
            false
        };

    } else {
        blockData[35] = {
            {
                glm::vec2(3.0f, 11.0f),
                glm::vec2(3.0f, 11.0f),
                glm::vec2(3.0f, 11.0f),
                glm::vec2(3.0f, 11.0f),
                glm::vec2(3.0f, 11.0f),
                glm::vec2(3.0f, 11.0f)
            },
            true,
            false,
            "Red Maple Leaves",
            "cube",
            true
        };

        blockData[36] = {
            {
                glm::vec2(4.0f, 11.0f),
                glm::vec2(4.0f, 11.0f),
                glm::vec2(4.0f, 11.0f),
                glm::vec2(4.0f, 11.0f),
                glm::vec2(4.0f, 11.0f),
                glm::vec2(4.0f, 11.0f)
            },
            true,
            false,
            "Orange Maple Leaves",
            "cube",
            true
        };

        blockData[37] = {
            {
                glm::vec2(5.0f, 11.0f),
                glm::vec2(5.0f, 11.0f),
                glm::vec2(5.0f, 11.0f),
                glm::vec2(5.0f, 11.0f),
                glm::vec2(5.0f, 11.0f),
                glm::vec2(5.0f, 11.0f)
            },
            true,
            false,
            "Yellow Maple Leaves",
            "cube",
            true
        };
    };

    blockData[38] = {
        {
            glm::vec2(0.0f, 11.0f),
            glm::vec2(0.0f, 11.0f),
            glm::vec2(0.0f, 11.0f),
            glm::vec2(0.0f, 11.0f),
            glm::vec2(1.0f, 11.0f),
            glm::vec2(1.0f, 11.0f)
        },
        false,
        false,
        "Maple Log",
        "cube",
        false
    };

    blockData[39] = {
        {
            glm::vec2(2.0f, 11.0f),
            glm::vec2(2.0f, 11.0f),
            glm::vec2(2.0f, 11.0f),
            glm::vec2(2.0f, 11.0f),
            glm::vec2(2.0f, 11.0f),
            glm::vec2(2.0f, 11.0f)
        },
        false,
        false,
        "Maple Plank",
        "cube",
        false
    };

    blockData[40] = {
        {
            glm::vec2(2.0f, 11.0f),
            glm::vec2(2.0f, 11.0f),
            glm::vec2(2.0f, 11.0f),
            glm::vec2(2.0f, 11.0f),
            glm::vec2(2.0f, 11.0f),
            glm::vec2(2.0f, 11.0f)
        },
        false,
        false,
        "Maple Plank Slab",
        "slab",
        false
    };

    blockData[41] = {
        {
            glm::vec2(5.0f, 14.0f),
            glm::vec2(5.0f, 14.0f),
            glm::vec2(5.0f, 14.0f),
            glm::vec2(5.0f, 14.0f),
            glm::vec2(5.0f, 14.0f),
            glm::vec2(5.0f, 14.0f)
        },
        false,
        false,
        "Leaves Carpet",
        "carpet",
        false
    };

    blockData[42] = {
        {
            glm::vec2(0.0f, 14.0f),
            glm::vec2(0.0f, 14.0f),
            glm::vec2(0.0f, 14.0f),
            glm::vec2(0.0f, 14.0f),
            glm::vec2(0.0f, 14.0f),
            glm::vec2(0.0f, 14.0f)
        },
        false,
        false,
        "Stone Bricks",
        "cube",
        false
    };

    blockData[43] = {
        {
            glm::vec2(0.0f, 14.0f),
            glm::vec2(0.0f, 14.0f),
            glm::vec2(0.0f, 14.0f),
            glm::vec2(0.0f, 14.0f),
            glm::vec2(0.0f, 14.0f),
            glm::vec2(0.0f, 14.0f)
        },
        false,
        false,
        "Stone Brick Slab",
        "slab",
        false
    };

    blockData[44] = {
        {
            glm::vec2(0.0f, 12.0f),
            glm::vec2(0.0f, 12.0f),
            glm::vec2(0.0f, 12.0f),
            glm::vec2(0.0f, 12.0f),
            glm::vec2(0.0f, 12.0f),
            glm::vec2(0.0f, 12.0f)
        },
        false,
        false,
        "Structure Air",
        "cube",
        false
    };
    
}

const BlockDB::BlockInfo* BlockDB::getBlockInfo(const uint8_t& blockName) {
    auto iterator = blockData.find(blockName);
    if (iterator != blockData.end()) {
        return &iterator->second;
    }
    return nullptr;
}
