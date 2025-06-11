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
    auto it = pendingBlockPlacements.find(key);
    if (it != pendingBlockPlacements.end()) {
        for (const auto& pb : it->second) {
            if (pb.x >= 0 && pb.x < WIDTH && pb.y >= 0 && pb.y < HEIGHT && pb.z >= 0 && pb.z < DEPTH) {
                blocks[pb.x][pb.y][pb.z].type = pb.type;
            }
        }
        pendingBlockPlacements.erase(it);
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
                int wx = baseX + x;
                int wy = baseY + y;
                int wz = baseZ + z;

                // Compute which chunk this block belongs to
                int chunkOffsetX = 0, chunkOffsetZ = 0;
                int localX = wx, localZ = wz;
                if (wx < 0) {
                    chunkOffsetX = (wx / WIDTH) - (wx % WIDTH != 0 ? 1 : 0);
                    localX = wx - chunkOffsetX * WIDTH;
                } else if (wx >= WIDTH) {
                    chunkOffsetX = wx / WIDTH;
                    localX = wx - chunkOffsetX * WIDTH;
                }
                if (wz < 0) {
                    chunkOffsetZ = (wz / DEPTH) - (wz % DEPTH != 0 ? 1 : 0);
                    localZ = wz - chunkOffsetZ * DEPTH;
                } else if (wz >= DEPTH) {
                    chunkOffsetZ = wz / DEPTH;
                    localZ = wz - chunkOffsetZ * DEPTH;
                }

                int targetChunkX = chunkX + chunkOffsetX;
                int targetChunkZ = chunkZ + chunkOffsetZ;

                if (wy >= 0 && wy < HEIGHT) {
                    Chunk* targetChunk = nullptr;
                    if (chunkOffsetX == 0 && chunkOffsetZ == 0) {
                        targetChunk = this;
                    } else if (world) {
                        targetChunk = world->getChunk(targetChunkX, targetChunkZ);
                    }
                    if (targetChunk &&
                        localX >= 0 && localX < WIDTH &&
                        localZ >= 0 && localZ < DEPTH) {
                        targetChunk->blocks[localX][wy][localZ].type = blockType;
                        affectedChunks.insert(targetChunk);
                    } else {
                        // Chunk not loaded, defer placement
                        auto key = std::make_pair(targetChunkX, targetChunkZ);
                        pendingBlockPlacements[key].push_back({localX, wy, localZ, blockType});
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
        int nx = 0, ny = 0, nz = 0;
        switch (face) {
            case 0: nx = chunkX;     ny = 0; nz = chunkZ + 1; break; // front
            case 1: nx = chunkX;     ny = 0; nz = chunkZ - 1; break; // back
            case 2: nx = chunkX - 1; ny = 0; nz = chunkZ;     break; // left
            case 3: nx = chunkX + 1; ny = 0; nz = chunkZ;     break; // right
            case 4: continue; // top face (no neighbor needed)
            case 5: continue; // bottom face (no neighbor needed)
        }
        if (world->getChunk(nx, nz) == nullptr) { // Neighbor chunk missing = skip mesh generation for now
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

    for (int x = 0; x < WIDTH; ++x) {
        for (int y = 0; y < HEIGHT; ++y) {
            for (int z = 0; z < DEPTH; ++z) {
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

    int nx = x + offsets[face][0];
    int ny = y + offsets[face][1];
    int nz = z + offsets[face][2];

    // Check height bounds
    if (ny < 0 || ny >= HEIGHT)
        return true;

    // If neighbor is within current chunk
    if (nx >= 0 && nx < WIDTH && nz >= 0 && nz < DEPTH) {
        return blocks[nx][ny][nz].type == 0;
    }

    // Neighbor is in another chunk
    int neighborChunkX = chunkX;
    int neighborChunkZ = chunkZ;
    int lx = nx;
    int lz = nz;

    if (lx < 0) {
        neighborChunkX -= 1;
        lx += WIDTH;
    } else if (lx >= WIDTH) {
        neighborChunkX += 1;
        lx -= WIDTH;
    }

    if (lz < 0) {
        neighborChunkZ -= 1;
        lz += DEPTH;
    } else if (lz >= DEPTH) {
        neighborChunkZ += 1;
        lz -= DEPTH;
    }

    Chunk* neighbor = world->getChunk(neighborChunkX, neighborChunkZ);
    if (!neighbor)
        return true;  // If no neighbor, assume empty

    return neighbor->blocks[lx][ny][lz].type == 0;
}

void Chunk::addFace(std::vector<float>& vertices, std::vector<unsigned int>& indices,
                    int x, int y, int z, int face, const BlockDB::BlockInfo* blockInfo, unsigned int& indexOffset) {
    static const glm::vec3 faceVertices[6][4] = {
        {{0,0,1}, {1,0,1}, {1,1,1}, {0,1,1}}, // Front
        {{1,0,0}, {0,0,0}, {0,1,0}, {1,1,0}}, // Back
        {{0,0,0}, {0,0,1}, {0,1,1}, {0,1,0}}, // Left
        {{1,0,1}, {1,0,0}, {1,1,0}, {1,1,1}}, // Right
        {{0,1,1}, {1,1,1}, {1,1,0}, {0,1,0}}, // Top
        {{0,0,0}, {1,0,0}, {1,0,1}, {0,0,1}}  // Bottom
    };

    static const glm::vec2 uvs[4] = {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f}
    };

    glm::vec2 texOffset = blockInfo->textureCoords[face] / 16.0f;

    for (int i = 0; i < 4; ++i) {
        glm::vec3 pos = faceVertices[face][i] + glm::vec3(x, y, z);
        glm::vec2 uv = (blockInfo->textureCoords[face] + uvs[i]) / 16.0f;
        vertices.insert(vertices.end(), {pos.x, pos.y, pos.z, uv.x, uv.y, static_cast<float>(face)});
    }

    indices.insert(indices.end(), {
        indexOffset, indexOffset + 1, indexOffset + 2,
        indexOffset + 2, indexOffset + 3, indexOffset
    });

    indexOffset += 4;
}

void Chunk::render(const Camera& camera, GLint uModelLoc) {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(chunkX * WIDTH, 0, chunkZ * DEPTH));
    glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, &model[0][0]);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}