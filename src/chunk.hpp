#pragma once

#include <vector>
#include <glad/glad.h>
#include "blockDB.hpp"
#include "camera.hpp"
#include "world.hpp"
#include "structureDB.hpp"

class World;

struct ChunkNoises;

class Chunk {
public:
    static const int WIDTH = 16;
    static const int HEIGHT = 256;
    static const int DEPTH = 16;

    enum class Biome {
        Plains,
        Desert,
        Forest
    };

    struct Block {
        uint8_t type;
    };

    Chunk(int x, int z, World* worldRef);
    ~Chunk();

    void generateTerrain();
    void buildMesh();
    void render(const Camera& camera, GLint uModelLoc);
    void placeStructure(const Structure& structure, int baseX, int baseY, int baseZ);

    Block blocks[WIDTH][HEIGHT][DEPTH];
    int chunkX, chunkZ;
    Biome biome;

private:
    World* world;

    GLuint VAO, VBO, EBO;
    GLsizei indexCount;

    void addFace(std::vector<float>& vertices, std::vector<unsigned int>& indices,
                 int x, int y, int z, int face, const BlockDB::BlockInfo* blockInfo, unsigned int& indexOffset);

    bool isBlockVisible(int x, int y, int z, int face) const;

    void generatePlainsFeatures();
    void generateDesertFeatures();
    void generateForestFeatures();

    friend class World; // Allow World to access private members (yay, a friend)
};
