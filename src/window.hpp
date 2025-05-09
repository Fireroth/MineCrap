#pragma once

#include <GLFW/glfw3.h>

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

private:
    int width;
    int height;
    const char* title;
    GLFWwindow* window;
};