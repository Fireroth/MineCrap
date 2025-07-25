#pragma once

#include <vector>
#include <glad/glad.h>
#include "blockDB.hpp"
#include "../core/camera.hpp"
#include "world.hpp"
#include "structureDB.hpp"
#include "noise.hpp"

class World;

class Chunk {
public:
    static const int chunkWidth = 16;
    static const int chunkHeight = 256;
    static const int chunkDepth = 16;

    ChunkNoises noises;

    enum class Biome {
        Plains,
        Desert,
        Forest,
        FirForest,
        FlowerField,
        MapleForest
    };

    struct Block {
        uint8_t type;
    };

    Chunk(int x, int z, World* worldPtr);
    ~Chunk();

    void buildMesh();
    void render(const Camera& camera, GLint uModelLoc);
    void renderCross(const Camera& camera, GLint uModelLoc);
    void renderLiquid(const Camera& camera, GLint uLiquidModelLoc);
    void placeStructure(const Structure& structure, int baseX, int baseY, int baseZ);

    Block blocks[chunkWidth][chunkHeight][chunkDepth];
    int chunkX, chunkZ;
    Biome biome;

private:
    World* world;

    GLuint VAO, VBO, EBO;
    GLuint crossVAO, crossVBO, crossEBO;
    GLuint liquidVAO, liquidVBO, liquidEBO;
    GLsizei indexCount;
    GLsizei crossIndexCount;
    GLsizei liquidIndexCount;

    void addFace(std::vector<float>& vertices, std::vector<unsigned int>& indices,
                 int x, int y, int z, int face, const BlockDB::BlockInfo* blockInfo, unsigned int& indexOffset);

    bool isBlockVisible(int x, int y, int z, int face) const;
};
