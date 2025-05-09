#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "camera.hpp"

void setupInputCallbacks(GLFWwindow* window, Camera* camera, class World* world);
void processInput(GLFWwindow* window, Camera& camera, float deltaTime, float speedMultiplier);
float getSpeedMultiplier(GLFWwindow* window);