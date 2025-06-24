#pragma once
#include <GLFW/glfw3.h>
#include "camera.hpp"

uint8_t getSelectedBlockType();
void setSelectedBlockType(uint8_t type);
void setupInputCallbacks(GLFWwindow* window, Camera* camera, class World* world);
void processInput(GLFWwindow* window, Camera& camera, float deltaTime, float speedMultiplier);
float getSpeedMultiplier(GLFWwindow* window);
bool getZoomState(GLFWwindow* window);