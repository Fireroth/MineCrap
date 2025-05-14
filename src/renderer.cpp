#include <glad/glad.h>
#include "renderer.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include <stb_image.h>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

Renderer::Renderer() : VAO(0), VBO(0), shaderProgram(0), textureAtlas(0), crosshairVAO(0), crosshairVBO(0) {}

Renderer::~Renderer()
{
    cleanup();
}

void Renderer::init()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    world.generateChunks(5);

    std::string vertexSource = loadShaderSource("shaders/vertex.glsl");
    std::string fragmentSource = loadShaderSource("shaders/fragment.glsl");
    shaderProgram = createShaderProgram(vertexSource.c_str(), fragmentSource.c_str());
    
    std::string crosshairVertexSource = loadShaderSource("shaders/crosshair_vertex.glsl");
    std::string crosshairFragmentSource = loadShaderSource("shaders/crosshair_fragment.glsl");
    crosshairShaderProgram = createShaderProgram(crosshairVertexSource.c_str(), crosshairFragmentSource.c_str());

    uModelLoc = glGetUniformLocation(shaderProgram, "model");
    uViewLoc = glGetUniformLocation(shaderProgram, "view");
    uProjLoc = glGetUniformLocation(shaderProgram, "projection");
    uAtlasLoc = glGetUniformLocation(shaderProgram, "atlas");
    uAspectLoc = glGetUniformLocation(crosshairShaderProgram, "aspectRatio");

    loadTextureAtlas("textures/atlas.png");
    initCrosshair();
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

void Renderer::renderWorld(const Camera& camera, float aspectRatio) {
    glUseProgram(shaderProgram);

    glm::mat4 projection = glm::perspective(glm::radians(60.0f), aspectRatio, 0.1f, 500.0f); // 500 = "view distance"
    glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, &camera.getViewMatrix()[0][0]);
    glUniformMatrix4fv(uProjLoc, 1, GL_FALSE, &projection[0][0]);


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureAtlas);
    glUniform1i(uAtlasLoc, 0);

    world.render(camera, uModelLoc);

    glDisable(GL_DEPTH_TEST);
    renderCrosshair(aspectRatio);
    glEnable(GL_DEPTH_TEST);
}

void Renderer::renderCrosshair(float aspectRatio) {
    glUseProgram(crosshairShaderProgram);
    
    if (uAspectLoc != -1) {
        glUniform1f(uAspectLoc, aspectRatio);
    }
    glBindVertexArray(crosshairVAO);

    glLineWidth(2.0f);
    glDrawArrays(GL_LINES, 0, 4);
    glLineWidth(1.0f);

    glBindVertexArray(0);
}

void Renderer::cleanup()
{
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    glDeleteTextures(1, &textureAtlas);
    glDeleteProgram(shaderProgram);

    if (crosshairVAO) glDeleteVertexArrays(1, &crosshairVAO);
    if (crosshairVBO) glDeleteBuffers(1, &crosshairVBO);
    glDeleteProgram(crosshairShaderProgram);
}

GLuint Renderer::createShader(const char *source, GLenum shaderType)
{
    return ::createShader(source, shaderType);
}

GLuint Renderer::createShaderProgram(const char *vertexSource, const char *fragmentSource)
{
    return ::createShaderProgram(vertexSource, fragmentSource);
}

void Renderer::loadTextureAtlas(const std::string& path)
{
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "Failed to load texture atlas: " << path << std::endl;
        return;
    }

    glGenTextures(1, &textureAtlas);
    glBindTexture(GL_TEXTURE_2D, textureAtlas);

    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    stbi_image_free(data);
}