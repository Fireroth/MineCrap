#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <set>
#include "chunk.hpp"
#include "../core/options.hpp"
#include "noise.hpp"
#include "chunkTerrain.hpp"

struct pendingBlock {
    int x, y, z;
    uint8_t type;
};
static std::map<std::pair<int, int>, std::vector<pendingBlock >> pendingBlockPlacements;

Chunk::Chunk(int x, int z, World* worldPtr)
    : chunkX(x), chunkZ(z), world(worldPtr), VAO(0), VBO(0), EBO(0), indexCount(0)
{
    noises = noiseInit();
    generateChunkTerrain(*this);

    // Apply any pending block placements for this chunk
    auto key = std::make_pair(chunkX, chunkZ);
    auto iterator = pendingBlockPlacements.find(key);
    if (iterator != pendingBlockPlacements.end()) {
        for (const auto& pb : iterator->second) {
            if (pb.x >= 0 && pb.x < chunkWidth && pb.y >= 0 && pb.y < chunkHeight && pb.z >= 0 && pb.z < chunkDepth) {
                blocks[pb.x][pb.y][pb.z].type = pb.type;
            }
        }
        pendingBlockPlacements.erase(iterator);
        buildMesh();
    }
}

Chunk::~Chunk() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Chunk::placeStructure(const Structure& structure, int baseX, int baseY, int baseZ) {
    int structHeight = (int)structure.layers.size();
    int structDepth = (int)structure.layers[0].size();
    int structWidth = (int)structure.layers[0][0].size();
    std::set<Chunk*> affectedChunks; // Track which chunks are affected

    for (int y = 0; y < structHeight; ++y) {
        for (int z = 0; z < structDepth; ++z) {
            for (int x = 0; x < structWidth; ++x) {
                uint8_t blockType = structure.layers[y][z][x];
                if (blockType == 0) continue;
                int worldX = baseX + x;
                int worldY = baseY + y;
                int worldZ = baseZ + z;

                // Compute which chunk this block belongs to
                int chunkOffsetX = 0, chunkOffsetZ = 0;
                int localX = worldX, localZ = worldZ;
                if (worldX < 0) {
                    chunkOffsetX = (worldX / chunkWidth) - (worldX % chunkWidth != 0 ? 1 : 0);
                    localX = worldX - chunkOffsetX * chunkWidth;
                } else if (worldX >= chunkWidth) {
                    chunkOffsetX = worldX / chunkWidth;
                    localX = worldX - chunkOffsetX * chunkWidth;
                }
                if (worldZ < 0) {
                    chunkOffsetZ = (worldZ / chunkDepth) - (worldZ % chunkDepth != 0 ? 1 : 0);
                    localZ = worldZ - chunkOffsetZ * chunkDepth;
                } else if (worldZ >= chunkDepth) {
                    chunkOffsetZ = worldZ / chunkDepth;
                    localZ = worldZ - chunkOffsetZ * chunkDepth;
                }

                int targetChunkX = chunkX + chunkOffsetX;
                int targetChunkZ = chunkZ + chunkOffsetZ;

                if (worldY >= 0 && worldY < chunkHeight) {
                    Chunk* targetChunk = nullptr;
                    if (chunkOffsetX == 0 && chunkOffsetZ == 0) {
                        targetChunk = this;
                    } else if (world) {
                        targetChunk = world->getChunk(targetChunkX, targetChunkZ);
                    }
                    if (targetChunk &&
                        localX >= 0 && localX < chunkWidth &&
                        localZ >= 0 && localZ < chunkDepth) {
                        targetChunk->blocks[localX][worldY][localZ].type = blockType;
                        affectedChunks.insert(targetChunk);
                    } else {
                        // Chunk not loaded, defer placement
                        auto key = std::make_pair(targetChunkX, targetChunkZ);
                        pendingBlockPlacements[key].push_back({localX, worldY, localZ, blockType});
                    }
                }
            }
        }
    }
    // Rebuild mesh for all affected chunks
    for (Chunk* chunk : affectedChunks) {
        chunk->buildMesh();
    }
}

void Chunk::buildMesh() {
    // Defer mesh generation if any neighbor chunk is missing
    for (int face = 0; face < 6; ++face) {
        int neighborX = 0, neighborY = 0, neighborZ = 0;
        switch (face) {
            case 0: neighborX = chunkX;     neighborY = 0; neighborZ = chunkZ + 1; break; // front
            case 1: neighborX = chunkX;     neighborY = 0; neighborZ = chunkZ - 1; break; // back
            case 2: neighborX = chunkX - 1; neighborY = 0; neighborZ = chunkZ;     break; // left
            case 3: neighborX = chunkX + 1; neighborY = 0; neighborZ = chunkZ;     break; // right
            case 4: continue; // top face (no neighbor needed)
            case 5: continue; // bottom face (no neighbor needed)
        }
        if (world->getChunk(neighborX, neighborZ) == nullptr) { // Neighbor chunk missing = skip mesh generation for now
            return;
        }
    }

    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
    }
    if (VBO != 0) {
        glDeleteBuffers(1, &VBO);
        VBO = 0;
    }
    if (EBO != 0) {
        glDeleteBuffers(1, &EBO);
        EBO = 0;
    }
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    unsigned int indexOffset = 0;

    for (int x = 0; x < chunkWidth; ++x) {
        for (int y = 0; y < chunkHeight; ++y) {
            for (int z = 0; z < chunkDepth; ++z) {
                const uint8_t& type = blocks[x][y][z].type;
                if (type == 0) continue;

                const BlockDB::BlockInfo* info = BlockDB::getBlockInfo(type);
                if (!info) continue;

                for (int face = 0; face < 6; ++face) {
                    if (isBlockVisible(x, y, z, face)) {
                        addFace(vertices, indices, x, y, z, face, info, indexOffset);
                    }
                }
            }
        }
    }

    indexCount = static_cast<GLsizei>(indices.size());

    // Create mesh
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Layout: position (3), uv (2), faceID (1)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

bool Chunk::isBlockVisible(int x, int y, int z, int face) const {
    static const int offsets[6][3] = {
        { 0,  0,  1},  // front
        { 0,  0, -1},  // back
        {-1,  0,  0},  // left
        { 1,  0,  0},  // right
        { 0,  1,  0},  // top
        { 0, -1,  0}   // bottom
    };

    int neighborX = x + offsets[face][0];
    int neighborY = y + offsets[face][1];
    int neighborZ = z + offsets[face][2];

    // Check height bounds
    if (neighborY < 0 || neighborY >= chunkHeight)
        return true;

    // If neighbor is within current chunk
    if (neighborX >= 0 && neighborX < chunkWidth && neighborZ >= 0 && neighborZ < chunkDepth) {
        //return blocks[neighborX][neighborY][neighborZ].type == 0;

        // ---- Temporary (maybe) ----
        uint8_t neighborType = blocks[neighborX][neighborY][neighborZ].type;
        if (neighborType == 0) return true;
        const BlockDB::BlockInfo* neighborInfo = BlockDB::getBlockInfo(neighborType);
        const BlockDB::BlockInfo* thisInfo = BlockDB::getBlockInfo(blocks[x][y][z].type);
        // Always show all faces if either block is leaves
        if (thisInfo->name == "Leaves")
            return true;

        // Always show all faces of non full block blocks
        if (!thisInfo->fullBlock)
            return true;

        // Always show face if neighbor is not a full block
        if (!neighborInfo->fullBlock)
            return true;

        // Render face if neighbor is transparent and current block is opaque
        if (neighborInfo->transparent && !thisInfo->transparent)
            return true;

        // Render face if neighbor is liquid and current block is non liquid
        if (neighborInfo->liquid && !thisInfo->liquid)
            return true;

        return false;
        // ---------------------------

    } else { // Neighbor is in another chunk
        int neighborChunkX = chunkX;
        int neighborChunkZ = chunkZ;
        int neighborLocalX = neighborX;
        int neighborLocalZ = neighborZ;

        if (neighborLocalX < 0) {
            neighborChunkX -= 1;
            neighborLocalX += chunkWidth;
        } else if (neighborLocalX >= chunkWidth) {
            neighborChunkX += 1;
            neighborLocalX -= chunkWidth;
        }

        if (neighborLocalZ < 0) {
            neighborChunkZ -= 1;
            neighborLocalZ += chunkDepth;
        } else if (neighborLocalZ >= chunkDepth) {
            neighborChunkZ += 1;
            neighborLocalZ -= chunkDepth;
        }

        Chunk* neighbor = world->getChunk(neighborChunkX, neighborChunkZ);
        if (!neighbor)
            return true;  // If no neighbor, assume empty

        //return neighbor->blocks[neighborLocalX][neighborY][neighborLocalZ].type == 0;
        // ---- Temporary (maybe) ----
        uint8_t neighborType = neighbor->blocks[neighborLocalX][neighborY][neighborLocalZ].type;
        if (neighborType == 0) return true;
        const BlockDB::BlockInfo* neighborInfo = BlockDB::getBlockInfo(neighborType);
        const BlockDB::BlockInfo* thisInfo = BlockDB::getBlockInfo(blocks[x][y][z].type);

        // Always show all faces if either block is leaves
        if (thisInfo->name == "leaves" || neighborInfo->name == "leaves")
            return true;

        // Always show all faces of non-full-block blocks
        if (!thisInfo->fullBlock)
            return true;

        // Always show face if neighbor is not a full block
        if (!neighborInfo->fullBlock)
            return true;

        // Render face if neighbor is transparent and current block is opaque
        if (neighborInfo->transparent && !thisInfo->transparent)
            return true;

        // Render face if neighbor is liquid and current block is non liquid
        if (neighborInfo->liquid && !thisInfo->liquid)
            return true;
            
        return false;
        // ---------------------------
    }
}

void Chunk::addFace(std::vector<float>& vertices, std::vector<unsigned int>& indices,
                    int x, int y, int z, int face, const BlockDB::BlockInfo* blockInfo, unsigned int& indexOffset) {
    static const glm::vec3 cubeFaceVertices[6][4] = {
        {{0,0,1}, {1,0,1}, {1,1,1}, {0,1,1}}, // Front
        {{1,0,0}, {0,0,0}, {0,1,0}, {1,1,0}}, // Back
        {{0,0,0}, {0,0,1}, {0,1,1}, {0,1,0}}, // Left
        {{1,0,1}, {1,0,0}, {1,1,0}, {1,1,1}}, // Right
        {{0,1,1}, {1,1,1}, {1,1,0}, {0,1,0}}, // Top
        {{0,0,0}, {1,0,0}, {1,0,1}, {0,0,1}}  // Bottom
    };

    static const glm::vec3 cactusFaceVertices[6][4] = {
        {{0.1,0,0.9}, {0.9,0,0.9}, {0.9,1,0.9}, {0.1,1,0.9}}, // Front
        {{0.9,0,0.1}, {0.1,0,0.1}, {0.1,1,0.1}, {0.9,1,0.1}}, // Back
        {{0.1,0,0.1}, {0.1,0,0.9}, {0.1,1,0.9}, {0.1,1,0.1}}, // Left
        {{0.9,0,0.9}, {0.9,0,0.1}, {0.9,1,0.1}, {0.9,1,0.9}}, // Right
        {{0.1,1,0.9}, {0.9,1,0.9}, {0.9,1,0.1}, {0.1,1,0.1}}, // Top
        {{0.1,0,0.1}, {0.9,0,0.1}, {0.9,0,0.9}, {0.1,0,0.9}}  // Bottom
    };

    static const glm::vec3 slabFaceVertices[6][4] = {
        {{0,0,1}, {1,0,1}, {1,0.5f,1}, {0,0.5f,1}}, // Front
        {{1,0,0}, {0,0,0}, {0,0.5f,0}, {1,0.5f,0}}, // Back
        {{0,0,0}, {0,0,1}, {0,0.5f,1}, {0,0.5f,0}}, // Left
        {{1,0,1}, {1,0,0}, {1,0.5f,0}, {1,0.5f,1}}, // Right
        {{0,0.5f,1}, {1,0.5f,1}, {1,0.5f,0}, {0,0.5f,0}}, // Top
        {{0,0,0}, {1,0,0}, {1,0,1}, {0,0,1}}  // Bottom
    };

    static const glm::vec3 liquidFaceVertices[6][4] = {
        {{0,0,1}, {1,0,1}, {1,0.85f,1}, {0,0.85f,1}}, // Front
        {{1,0,0}, {0,0,0}, {0,0.85f,0}, {1,0.85f,0}}, // Back
        {{0,0,0}, {0,0,1}, {0,0.85f,1}, {0,0.85f,0}}, // Left
        {{1,0,1}, {1,0,0}, {1,0.85f,0}, {1,0.85f,1}}, // Right
        {{0,0.85f,1}, {1,0.85f,1}, {1,0.85f,0}, {0,0.85f,0}}, // Top
        {{0,0,0}, {1,0,0}, {1,0,1}, {0,0,1}}  // Bottom
    };

    static const glm::vec2 cubeUvs[4] = {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f}
    };

    static const glm::vec2 slabUvs[4] = {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 0.5f},
        {0.0f, 0.5f}
    };

    static const glm::vec2 liquidUvs[4] = {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 0.85f},
        {0.0f, 0.85f}
    };

    // Choose face vertices and uvs based on block parameters
    const glm::vec3* usedFaceVerts;
    const glm::vec2* usedUvs;
    if (blockInfo->modelName == "cactus") {
        usedFaceVerts = cactusFaceVertices[face];
        usedUvs = cubeUvs;
    } else if (blockInfo->liquid) {
        usedFaceVerts = liquidFaceVertices[face];
        // Use "liquidUvs" for side faces, "cubeUvs" for top/bottom
        if (face >= 0 && face <= 3) {
            usedUvs = liquidUvs;
        } else {
            usedUvs = cubeUvs;
        }
    } else if (blockInfo->modelName == "slab") {
        usedFaceVerts = slabFaceVertices[face];
        // Use "slabUvs" for side faces, "cubeUvs" for top/bottom
        if (face >= 0 && face <= 3) {
            usedUvs = slabUvs;
        } else {
            usedUvs = cubeUvs;
        }
    } else {
        usedFaceVerts = cubeFaceVertices[face];
        usedUvs = cubeUvs;
    }

    glm::vec2 texOffset = blockInfo->textureCoords[face] / 16.0f;

    for (int i = 0; i < 4; ++i) {
        glm::vec3 pos = usedFaceVerts[i] + glm::vec3(x, y, z);
        glm::vec2 uv = (blockInfo->textureCoords[face] + usedUvs[i]) / 16.0f;
        vertices.insert(vertices.end(), {pos.x, pos.y, pos.z, uv.x, uv.y, static_cast<float>(face)});
    }

    indices.insert(indices.end(), {
        indexOffset, indexOffset + 1, indexOffset + 2,
        indexOffset + 2, indexOffset + 3, indexOffset
    });

    indexOffset += 4;
}

void Chunk::render(const Camera& camera, GLint uModelLoc) {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(chunkX * chunkWidth, 0, chunkZ * chunkDepth));
    glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, &model[0][0]);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}