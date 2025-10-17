#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include "renderer.hpp"
#include "shader.hpp"
#include "../core/camera.hpp"
#include "../core/options.hpp"

Renderer::Renderer() : shaderProgram(0), textureAtlas(0), crosshairVAO(0), crosshairVBO(0) {}

Renderer::~Renderer() {
    glDeleteTextures(1, &textureAtlas);
    glDeleteProgram(shaderProgram);

    glDeleteVertexArrays(1, &crosshairVAO);
    glDeleteBuffers(1, &crosshairVBO);
    glDeleteProgram(crosshairShaderProgram);
}

void Renderer::init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(2.0f);

    world.generateChunks(2); // Generate initial chunks around the 0,0

    std::string vertexSource = loadShaderSource("shaders/vertex.glsl");
    std::string fragmentSource = loadShaderSource("shaders/fragment.glsl");
    shaderProgram = createShaderProgram(vertexSource.c_str(), fragmentSource.c_str());

    std::string crossVertexSource = loadShaderSource("shaders/cross_vertex.glsl");
    std::string crossFragmentSource = loadShaderSource("shaders/cross_fragment.glsl");
    crossShaderProgram = createShaderProgram(crossVertexSource.c_str(), crossFragmentSource.c_str());

    std::string liquidVertexSource = loadShaderSource("shaders/liquid_vertex.glsl");
    std::string liquidFragmentSource = loadShaderSource("shaders/liquid_fragment.glsl");
    liquidShaderProgram = createShaderProgram(liquidVertexSource.c_str(), liquidFragmentSource.c_str());
    
    std::string crosshairVertexSource = loadShaderSource("shaders/crosshair_vertex.glsl");
    std::string crosshairFragmentSource = loadShaderSource("shaders/crosshair_fragment.glsl");
    crosshairShaderProgram = createShaderProgram(crosshairVertexSource.c_str(), crosshairFragmentSource.c_str());

    uCrosshairAspectLoc = glGetUniformLocation(crosshairShaderProgram, "aspectRatio");

    uCrossModelLoc = glGetUniformLocation(crossShaderProgram, "model");
    uCrossViewLoc = glGetUniformLocation(crossShaderProgram, "view");
    uCrossProjLoc = glGetUniformLocation(crossShaderProgram, "projection");
    uCrossAtlasLoc = glGetUniformLocation(crossShaderProgram, "atlas");
    uCrossFogDensityLoc = glGetUniformLocation(crossShaderProgram, "fogDensity");
    uCrossFogStartLoc = glGetUniformLocation(crossShaderProgram, "fogStartDistance");
    uCrossFogColorLoc = glGetUniformLocation(crossShaderProgram, "fogColor");
    uCrossCamPosLoc = glGetUniformLocation(crossShaderProgram, "cameraPos");

    uLiquidModelLoc = glGetUniformLocation(liquidShaderProgram, "model");
    uLiquidViewLoc = glGetUniformLocation(liquidShaderProgram, "view");
    uLiquidProjLoc = glGetUniformLocation(liquidShaderProgram, "projection");
    uLiquidAtlasLoc = glGetUniformLocation(liquidShaderProgram, "atlas");
    uLiquidTimeLoc = glGetUniformLocation(liquidShaderProgram, "time");
    uLiquidFogDensityLoc = glGetUniformLocation(liquidShaderProgram, "fogDensity");
    uLiquidFogStartLoc = glGetUniformLocation(liquidShaderProgram, "fogStartDistance");
    uLiquidFogColorLoc = glGetUniformLocation(liquidShaderProgram, "fogColor");
    uLiquidCamPosLoc = glGetUniformLocation(liquidShaderProgram, "cameraPos");

    uModelLoc = glGetUniformLocation(shaderProgram, "model");
    uViewLoc = glGetUniformLocation(shaderProgram, "view");
    uProjLoc = glGetUniformLocation(shaderProgram, "projection");
    uAtlasLoc = glGetUniformLocation(shaderProgram, "atlas");
    uFogDensityLoc = glGetUniformLocation(shaderProgram, "fogDensity");
    uFogStartLoc = glGetUniformLocation(shaderProgram, "fogStartDistance");
    uFogColorLoc = glGetUniformLocation(shaderProgram, "fogColor");
    uCamPosLoc = glGetUniformLocation(shaderProgram, "cameraPos");

    loadTextureAtlas("textures/atlas.png");
    initCrosshair();

    currentFov = getOptionFloat("fov", 60.0f);

    fogEnabled = getOptionInt("fog", 1);
    fogDensity = 0.30f;
    fogStartDistance = ((getOptionFloat("render_distance", 7) + 1) * 16) - 20;
    fogColor = glm::vec3(0.6f, 1.0f, 1.0f);
}

void Renderer::initCrosshair() {
    float crosshairVertices[] = {
        -0.025f,  0.0f,
         0.025f,  0.0f,
         0.0f,  -0.025f,
         0.0f,   0.025f
    };

    glGenVertexArrays(1, &crosshairVAO);
    glGenBuffers(1, &crosshairVBO);

    glBindVertexArray(crosshairVAO);

    glBindBuffer(GL_ARRAY_BUFFER, crosshairVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(crosshairVertices), crosshairVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void Renderer::renderWorld(const Camera& camera, float aspectRatio, float deltaTime, float currentFrame) {
    glEnable(GL_DEPTH_TEST);
    int renderDist = getOptionInt("render_distance", 7) + 1; // +1 to account for invisible "mesh helper" chunk
    fogStartDistance = ((getOptionFloat("render_distance", 7) + 1) * 16) - 20;
    world.updateChunksAroundPlayer(camera.getPosition(), renderDist);

    GLFWwindow* getCurrentGLFWwindow();
    GLFWwindow* window = getCurrentGLFWwindow();
    float baseFov = getOptionFloat("fov", 60.0f);
    
    float getSpeedMultiplier(GLFWwindow* window);
    bool sprintState = window && getSpeedMultiplier(window) > 2.4f;
    float sprintFov = baseFov + 10.0f;

    bool getZoomState(GLFWwindow* window);
    bool zoomState = window && getZoomState(window);
    float zoomFov = baseFov * 0.1f;

    float targetFov = baseFov;
    if (zoomState) {
        targetFov = zoomFov;
    } else if (sprintState) {
        targetFov = sprintFov;
    }

    float fovLerpSpeed = 20.0f;
    float lerpFactor = 1.0f - expf(-fovLerpSpeed * deltaTime);
    currentFov = currentFov + (targetFov - currentFov) * lerpFactor;

    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(currentFov), aspectRatio, 0.1f, 5000.0f);
    
    Frustum frustum = World::extractFrustumPlanes(projection * view);

    // -------------------------------- Render main --------------------------------

    glUseProgram(shaderProgram);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    
    glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(uProjLoc, 1, GL_FALSE, &projection[0][0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureAtlas);
    glUniform1i(uAtlasLoc, 0);

    if (uCamPosLoc != -1) {
        glm::vec3 camPos = camera.getPosition();
        glUniform3fv(uCamPosLoc, 1, glm::value_ptr(camPos));
    }

    if (fogEnabled) {
        glUniform1f(uFogDensityLoc, fogDensity);
        glUniform1f(uFogStartLoc, fogStartDistance);
        glUniform3fv(uFogColorLoc, 1, glm::value_ptr(fogColor));
    } else {
        glUniform1f(uFogDensityLoc, 0.0f); // Disable fog
    }

    world.render(camera, uModelLoc, frustum);

    // -------------------------------- Render cross --------------------------------

    glUseProgram(crossShaderProgram);
    glDisable(GL_CULL_FACE);

    glUniformMatrix4fv(uCrossViewLoc, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(uCrossProjLoc, 1, GL_FALSE, &projection[0][0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureAtlas);
    glUniform1i(uCrossAtlasLoc, 0);

    if (uCrossCamPosLoc != -1) {
        glm::vec3 camPos = camera.getPosition();
        glUniform3fv(uCrossCamPosLoc, 1, glm::value_ptr(camPos));
    }

    if (fogEnabled) {
        glUniform1f(uCrossFogDensityLoc, fogDensity);
        glUniform1f(uCrossFogStartLoc, fogStartDistance);
        glUniform3fv(uCrossFogColorLoc, 1, glm::value_ptr(fogColor));
    } else {
        glUniform1f(uCrossFogDensityLoc, 0.0f); // Disable fog
    }
    
    world.renderCross(camera, uCrossModelLoc, frustum);

    // -------------------------------- Render liquid --------------------------------

    glUseProgram(liquidShaderProgram);

    glUniformMatrix4fv(uLiquidViewLoc, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(uLiquidProjLoc, 1, GL_FALSE, &projection[0][0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureAtlas);
    glUniform1i(uLiquidAtlasLoc, 0);
    glUniform1f(uLiquidTimeLoc, currentFrame);

    if (uLiquidCamPosLoc != -1) {
        glm::vec3 camPos = camera.getPosition();
        glUniform3fv(uLiquidCamPosLoc, 1, glm::value_ptr(camPos));
    }

    if (fogEnabled) {
        glUniform1f(uLiquidFogDensityLoc, fogDensity);
        glUniform1f(uLiquidFogStartLoc, fogStartDistance);
        glUniform3fv(uLiquidFogColorLoc, 1, glm::value_ptr(fogColor));
    } else {
        glUniform1f(uLiquidFogDensityLoc, 0.0f); // Disable fog
    }

    world.renderLiquid(camera, uLiquidModelLoc, frustum);

    glDisable(GL_BLEND);
}

void Renderer::renderCrosshair(float aspectRatio) {
    glDisable(GL_DEPTH_TEST);
    glUseProgram(crosshairShaderProgram);
    
    if (uCrosshairAspectLoc != -1) {
        glUniform1f(uCrosshairAspectLoc, aspectRatio);
    }
    glBindVertexArray(crosshairVAO);
    glDrawArrays(GL_LINES, 0, 4);
    glBindVertexArray(0);
}

GLuint Renderer::createShader(const char *source, GLenum shaderType) {
    return ::createShader(source, shaderType);
}

GLuint Renderer::createShaderProgram(const char *vertexSource, const char *fragmentSource) {
    return ::createShaderProgram(vertexSource, fragmentSource);
}

void Renderer::loadTextureAtlas(const std::string& path) {
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 4);
    if (!data) {
        std::cerr << "Failed to load texture atlas: " << path << std::endl;
        return;
    }

    glGenTextures(1, &textureAtlas);
    glBindTexture(GL_TEXTURE_2D, textureAtlas);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    stbi_image_free(data);
}