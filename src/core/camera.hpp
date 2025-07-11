#pragma once

#include <glm/glm.hpp>

class Camera {
public:
    Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch);

    glm::mat4 getViewMatrix() const;
    void processKeyboard(const char* direction, float deltaTime, float speedMultiplier = 1.0f);
    void processMouseMovement(float xOffset, float yOffset);

    glm::vec3 getPosition() const { return position; }
    glm::vec3 getFront() const { return front; }
    glm::vec3 getUp() const { return up; }
    float getYaw() const { return yaw; }
    float getPitch() const { return pitch; }

    void updateVelocity(float deltaTime);
    void applyAcceleration(const glm::vec3& acceleration, float deltaTime);

private:
    void updateCameraVectors();

    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    float yaw;
    float pitch;

    float movementSpeed;
    float mouseSensitivity;

    glm::vec3 velocity = glm::vec3(0.0f);
};