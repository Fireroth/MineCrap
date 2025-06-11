#include <glad/glad.h>
#include <iostream>
#include "window.hpp"
#include "stb_image.h"

Window::Window(int width, int height, const char* title)
    : width(width), height(height), title(title), window(nullptr) {}

Window::~Window() {
    cleanup();
}

void Window::framebufferSizeCallback(GLFWwindow* glfwWindow, int width, int height) {
    glViewport(0, 0, width, height);
    std::cout << "Resolution changed: " << width << "x" << height << std::endl;

    Window* window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
    if (window) {
        window->width = width;
        window->height = height;
        window->aspectRatio = static_cast<float>(width) / static_cast<float>(height);
        if (window->framebufferResizeCallback) {
            window->framebufferResizeCallback(width, height, window->aspectRatio);
        }
    }
}

bool Window::init() {
    if (!glfwInit()) return false;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return false;
    }

    setIcon("textures/icon.png");

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    aspectRatio = static_cast<float>(width) / static_cast<float>(height);

    return true;
}

void Window::pollEvents() const {
    glfwPollEvents();
}

void Window::clear(float r, float g, float b, float a) const {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::swapBuffers() const {
    glfwSwapBuffers(window);
}

void Window::cleanup() {
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(window);
}

GLFWwindow* Window::getGLFWwindow() const {
    return window;
}

void Window::setIcon(const char* iconPath) {
    GLFWimage image;
    int channels;
    unsigned char* pixels = stbi_load(iconPath, &image.width, &image.height, &channels, 4);
    
    if (pixels) {
        image.pixels = pixels;
        glfwSetWindowIcon(window, 1, &image);
        stbi_image_free(pixels);
    } else {
        std::cerr << "Failed to load window icon: " << iconPath << std::endl;
    }
}

float Window::getAspectRatio() const {
    return aspectRatio;
}

void Window::setFramebufferResizeCallback(std::function<void(int, int, float)> callback) {
    framebufferResizeCallback = callback;
}