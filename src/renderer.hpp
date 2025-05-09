#pragma once

#include <string>
#include "world.hpp"

class Renderer {
public:
    Renderer();
    ~Renderer();

    void init();
    void cleanup();
    void renderWorld(const class Camera& camera, float aspectRatio);

    World world;

private:
    GLuint VAO, VBO, EBO, shaderProgram, textureAtlas;
    World* currentWorld = nullptr;
    GLuint createShader(const char* source, GLenum shaderType);
    GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource);
    void loadTextureAtlas(const std::string& path);
};


