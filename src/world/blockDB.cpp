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
        "Grass Block",  // Name
        "cube",         // Model
        false,          // Render faces in between
        "Blocks"        // Tab name
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
        false,
        "Blocks"
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
        false,
        "Blocks"
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
        false,
        "Blocks"
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
        false,
        "Blocks"
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
        false,
        "Blocks"
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
        false,
        "Blocks"
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
        false,
        "Blocks"
    };

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
        false,
        "Liquids"
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
        "liquid",
        false,
        "Liquids"
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
            false,
            "Blocks"
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
            true,
            "Blocks"
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
        false,
        "Blocks"
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
        false,
        "Slabs"
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
        false,
        "Blocks"
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
        false,
        "Slabs"
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
        false,
        "Blocks"
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
        false,
        "Plants"
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
        false,
        "Plants"
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
        false,
        "Plants"
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
        false,
        "Plants"
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
        false,
        "Plants"
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
            true,
            "Blocks"
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
            true,
            "Blocks"
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
        false,
        "Blocks"
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
        false,
        "Plants"
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
        false,
        "Blocks"
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
        false,
        "Plants"
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
        false,
        "Blocks"
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
        false,
        "Plants"
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
        false,
        "Plants"
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
        false,
        "Plants"
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
        false,
        "Blocks"
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
        false,
        "Slabs"
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
        false,
        "Plants"
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
        false,
        "Plants"
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
            false,
            "Blocks"
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
            false,
            "Blocks"
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
            false,
            "Blocks"
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
            true,
            "Blocks"
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
            true,
            "Blocks"
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
            true,
            "Blocks"
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
        false,
        "Blocks"
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
        false,
        "Blocks"
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
        false,
        "Slabs"
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
        false,
        "Blocks"
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
        false,
        "Blocks"
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
        false,
        "Slabs"
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
        false,
        "Internal"
    };

    blockData[45] = {
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
        "Sand Slab",
        "slab",
        false,
        "Slabs"
    };

    blockData[46] = {
        {
            glm::vec2(2.0f, 15.0f),
            glm::vec2(2.0f, 15.0f),
            glm::vec2(2.0f, 15.0f),
            glm::vec2(2.0f, 15.0f),
            glm::vec2(2.0f, 15.0f),
            glm::vec2(1.0f, 15.0f)
        },
        false,
        false,
        "Grass Block Slab",
        "slab",
        false,
        "Slabs"
    };

    blockData[47] = {
        {
            glm::vec2(2.0f, 12.0f),
            glm::vec2(2.0f, 12.0f),
            glm::vec2(2.0f, 12.0f),
            glm::vec2(2.0f, 12.0f),
            glm::vec2(2.0f, 12.0f),
            glm::vec2(1.0f, 15.0f)
        },
        false,
        false,
        "Dark Grass Block Slab",
        "slab",
        false,
        "Slabs"
    };

    blockData[48] = {
        {
            glm::vec2(2.0f, 15.0f),
            glm::vec2(2.0f, 15.0f),
            glm::vec2(2.0f, 15.0f),
            glm::vec2(2.0f, 15.0f),
            glm::vec2(2.0f, 15.0f),
            glm::vec2(1.0f, 15.0f)
        },
        false,
        false,
        "Grass Block (all sides)",
        "cube",
        false,
        "Blocks"
    };

    blockData[49] = {
        {
            glm::vec2(2.0f, 12.0f),
            glm::vec2(2.0f, 12.0f),
            glm::vec2(2.0f, 12.0f),
            glm::vec2(2.0f, 12.0f),
            glm::vec2(2.0f, 12.0f),
            glm::vec2(1.0f, 15.0f)
        },
        false,
        false,
        "Dark Grass Block (all sides)",
        "cube",
        false,
        "Blocks"
    };

    blockData[50] = {
        {
            glm::vec2(9.0f, 15.0f),
            glm::vec2(9.0f, 15.0f),
            glm::vec2(9.0f, 15.0f),
            glm::vec2(9.0f, 15.0f),
            glm::vec2(9.0f, 15.0f),
            glm::vec2(9.0f, 15.0f)
        },
        false,
        false,
        "Sand Bricks",
        "cube",
        false,
        "Blocks"
    };

    if (fasterTrees) {
        blockData[51] = {
            {
                glm::vec2(2.0f, 10.0f),
                glm::vec2(2.0f, 10.0f),
                glm::vec2(2.0f, 10.0f),
                glm::vec2(2.0f, 10.0f),
                glm::vec2(2.0f, 10.0f),
                glm::vec2(2.0f, 10.0f)
            },
            false,
            false,
            "Green Maple Leaves",
            "cube",
            false,
            "Blocks"
        };

    } else {
        blockData[51] = {
            {
                glm::vec2(1.0f, 10.0f),
                glm::vec2(1.0f, 10.0f),
                glm::vec2(1.0f, 10.0f),
                glm::vec2(1.0f, 10.0f),
                glm::vec2(1.0f, 10.0f),
                glm::vec2(1.0f, 10.0f)
            },
            true,
            false,
            "Green Maple Leaves",
            "cube",
            true,
            "Blocks"
        };
    };

    blockData[52] = {
        {
            glm::vec2(1.0f, 10.0f),
            glm::vec2(1.0f, 10.0f),
            glm::vec2(1.0f, 10.0f),
            glm::vec2(1.0f, 10.0f),
            glm::vec2(1.0f, 10.0f),
            glm::vec2(1.0f, 10.0f)
        },
        false,
        false,
        "Green Maple Leaves Carpet",
        "carpet",
        false,
        "Blocks"
    };
    
}

const BlockDB::BlockInfo* BlockDB::getBlockInfo(const uint8_t& blockName) {
    auto iterator = blockData.find(blockName);
    if (iterator != blockData.end()) {
        return &iterator->second;
    }
    return nullptr;
}
