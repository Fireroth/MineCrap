#pragma once

#include <string>
#include "../world/world.hpp"

class Renderer {
public:
    GLint uModelLoc, uViewLoc, uProjLoc, uAtlasLoc, uAspectLoc, uFogDensityLoc, uFogStartLoc, uFogColorLoc;
    Renderer();
    ~Renderer();

    void init();
    void cleanup();
    void renderWorld(const class Camera& camera, float aspectRatio, float deltaTime);
    void renderCrosshair(float aspectRatio);

    World world;
    float currentFov;
    bool fogEnabled;
    float fogDensity;
    float fogStartDistance;
    glm::vec3 fogColor;

private:
    GLuint shaderProgram, textureAtlas;
    GLuint crosshairVAO, crosshairVBO, crosshairShaderProgram;
    World* currentWorld = nullptr;
    GLuint createShader(const char* source, GLenum shaderType);
    GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource);
    void loadTextureAtlas(const std::string& path);
    void initCrosshair();
};
