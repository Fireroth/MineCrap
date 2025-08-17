#include <glm/gtc/matrix_transform.hpp>
#include "camera.hpp"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : position(position), worldUp(up), yaw(yaw), pitch(pitch), movementSpeed(2.5f), mouseSensitivity(0.1f) {
    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, position + front, up);
}

void Camera::processKeyboard(const char *direction, float deltaTime, float speedMultiplier) {
    float acceleration = movementSpeed * speedMultiplier * 11.0f;
    glm::vec3 accel(0.0f);

    if (strcmp(direction, "FORWARD") == 0)
        accel += glm::normalize(glm::vec3(front.x, 0.0f, front.z)) * acceleration;
    if (strcmp(direction, "BACKWARD") == 0)
        accel -= glm::normalize(glm::vec3(front.x, 0.0f, front.z)) * acceleration;
    if (strcmp(direction, "LEFT") == 0)
        accel -= right * acceleration;
    if (strcmp(direction, "RIGHT") == 0)
        accel += right * acceleration;
    if (strcmp(direction, "UP") == 0)
        accel += worldUp * acceleration;
    if (strcmp(direction, "DOWN") == 0)
        accel -= worldUp * acceleration;

    applyAcceleration(accel, deltaTime);
}

void Camera::processMouseMovement(float xOffset, float yOffset) {
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

void Camera::updateCameraVectors() {
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}

void Camera::updateVelocity(float deltaTime) {
    position += velocity * deltaTime;

    float drag = 9.0f;
    velocity -= velocity * glm::min(drag * deltaTime, 1.0f);

    if (glm::length(velocity) < 0.01f)
        velocity = glm::vec3(0.0f);
}

void Camera::applyAcceleration(const glm::vec3& acceleration, float deltaTime) {
    velocity += acceleration * deltaTime;
}