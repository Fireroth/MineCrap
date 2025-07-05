#pragma once

#include <string>
#include "../world/world.hpp"

class Renderer {
public:
    GLint uModelLoc, uViewLoc, uProjLoc, uAtlasLoc, uAspectLoc, uFogDensityLoc, uFogStartLoc, uFogColorLoc, uCamPosLoc;
    GLint uCrossModelLoc, uCrossViewLoc, uCrossProjLoc, uCrossAtlasLoc;
    GLint uLiquidModelLoc, uLiquidViewLoc, uLiquidProjLoc, uLiquidAtlasLoc;
    GLint uLiquidTimeLoc, uCrossTimeLoc, uTimeLoc;
    Renderer();
    ~Renderer();

    void init();
    void renderWorld(const class Camera& camera, float aspectRatio, float deltaTime, float currentFrame);
    void renderCrosshair(float aspectRatio);

    World world;
    float currentFov;
    bool fogEnabled;
    float fogDensity;
    float fogStartDistance;
    glm::vec3 fogColor;

private:
    GLuint shaderProgram, textureAtlas;
    GLuint crossShaderProgram;
    GLuint liquidShaderProgram;
    GLuint crosshairVAO, crosshairVBO, crosshairShaderProgram;
    GLuint createShader(const char* source, GLenum shaderType);
    GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource);
    void loadTextureAtlas(const std::string& path);
    void initCrosshair();
};
