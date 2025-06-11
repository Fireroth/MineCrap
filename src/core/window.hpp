#pragma once

#include <GLFW/glfw3.h>
#include <functional>

class Window {
public:
    Window(int width, int height, const char* title);
    ~Window();

    bool init();
    void pollEvents() const;
    void clear(float r, float g, float b, float a) const;
    void swapBuffers() const;
    void cleanup();
    bool shouldClose() const;
    void setIcon(const char* iconPath);

    GLFWwindow* getGLFWwindow() const;

    float getAspectRatio() const;
    void setFramebufferResizeCallback(std::function<void(int, int, float)> callback);

private:
    int width;
    int height;
    const char* title;
    GLFWwindow* window;

    float aspectRatio = 1.0f;
    std::function<void(int, int, float)> framebufferResizeCallback;
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
};