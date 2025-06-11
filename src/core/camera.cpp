#include <glm/gtc/matrix_transform.hpp>
#include "camera.hpp"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : position(position), worldUp(up), yaw(yaw), pitch(pitch), movementSpeed(2.5f), mouseSensitivity(0.1f)
{
    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(position, position + front, up);
}

void Camera::processKeyboard(const char *direction, float deltaTime, float speedMultiplier)
{
    float velocity = movementSpeed * deltaTime * speedMultiplier;
    if (strcmp(direction, "FORWARD") == 0)
        position += glm::normalize(glm::vec3(front.x, 0.0f, front.z)) * velocity; // Ignore Y
    if (strcmp(direction, "BACKWARD") == 0)
        position -= glm::normalize(glm::vec3(front.x, 0.0f, front.z)) * velocity; // Ignore Y
    if (strcmp(direction, "LEFT") == 0)
        position -= right * velocity;
    if (strcmp(direction, "RIGHT") == 0)
        position += right * velocity;
    if (strcmp(direction, "UP") == 0)
        position += worldUp * velocity;
    if (strcmp(direction, "DOWN") == 0)
        position -= worldUp * velocity;
}

void Camera::processMouseMovement(float xOffset, float yOffset)
{
    xOffset *= mouseSensitivity;
    yOffset *= mouseSensitivity;

    yaw += xOffset;
    pitch += yOffset;

    // Avoid screen flipping
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    if (yaw > 180.0f) yaw = -180.0f;
    if (yaw < -180.0f) yaw = 180.0f;

    updateCameraVectors();
}

void Camera::updateCameraVectors()
{
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}