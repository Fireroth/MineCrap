#pragma once

#include <glm/glm.hpp>

class Camera {
public:
    Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch);

    glm::mat4 getViewMatrix() const;
    void processKeyboard(const char* direction, float deltaTime, float speedMultiplier = 1.0f);
    void processMouseMovement(float xOffset, float yOffset);

    glm::dvec3 getPositionDouble() const { return position; }
    glm::vec3 getPosition() const { return glm::vec3(position); }
    glm::vec3 getFront() const { return front; }
    glm::vec3 getUp() const { return up; }
    glm::dvec3 getVelocity() const { return velocity; }
    float getYaw() const { return yaw; }
    float getPitch() const { return pitch; }
    float getPlayerRadius() const { return playerRadius; }
    float getPlayerHeight() const { return playerHeight; }
    float getEyeHeight() const { return eyeHeight; }

    void updateVelocity(float deltaTime, class World* world = nullptr);
    void stepVelocity(float deltaTime, class World* world);
    void updateVelocityFlight(float deltaTime);
    void applyAcceleration(const glm::vec3& acceleration, float deltaTime);
    void jump();
    bool isGrounded() const { return grounded; }

    void setPosition(const glm::dvec3& pos);

private:
    void updateCameraVectors();

    glm::dvec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    float yaw;
    float pitch;

    float movementSpeed;
    float mouseSensitivity;

    glm::dvec3 velocity = glm::dvec3(0.0);
    bool grounded = false;

    float gravity = -30.0f;
    float jumpPower = 9.0f;
    float playerHeight = 1.8f;
    float eyeHeight = 1.67f;
    float playerRadius = 0.3f;
    float stepHeight = 0.51f;

    float coyoteTime = 0.12f; // seconds we allow jump after walking off an edge
    float coyoteTimer = 0.0f;
    float jumpBufferTime = 0.1f; // seconds to buffer a jump input
    float jumpBufferTimer = 0.0f;
    bool jumpBuffered = false;
};