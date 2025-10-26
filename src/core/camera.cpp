#include <glm/gtc/matrix_transform.hpp>
#include <cstring>
#include "camera.hpp"
#include "../world/world.hpp"
#include "../world/chunk.hpp"
#include "../world/blockDB.hpp"

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

// Helper: get AABB for block id
static void getBlockAABB(uint8_t blockId, glm::vec3& outMin, glm::vec3& outMax) {
    const BlockDB::BlockInfo* info = BlockDB::getBlockInfo(blockId);
    if (!info) { outMin = glm::vec3(0.0f); outMax = glm::vec3(1.0f); return; }
    if (info->modelName == "cactus") {
        outMin = glm::vec3(0.1f, 0.0f, 0.1f);
        outMax = glm::vec3(0.9f, 1.0f, 0.9f);
    } else if (info->modelName == "slab") {
        outMin = glm::vec3(0.0f, 0.0f, 0.0f);
        outMax = glm::vec3(1.0f, 0.5f, 1.0f);
    } else if (info->modelName == "carpet") {
        outMin = glm::vec3(0.0f, 0.0f, 0.0f);
        outMax = glm::vec3(1.0f, 0.05f, 1.0f);
    } else {
        outMin = glm::vec3(0.0f);
        outMax = glm::vec3(1.0f);
    }
}

static const double COLLISION_EPS = 1e-6;
static bool aabbOverlap(const glm::dvec3& amin, const glm::dvec3& amax, const glm::dvec3& bmin, const glm::dvec3& bmax) {
    return (amin.x <= bmax.x - COLLISION_EPS && amax.x >= bmin.x + COLLISION_EPS) &&
           (amin.y <= bmax.y - COLLISION_EPS && amax.y >= bmin.y + COLLISION_EPS) &&
           (amin.z <= bmax.z - COLLISION_EPS && amax.z >= bmin.z + COLLISION_EPS);
}

static bool aabbOverlapStrict(const glm::dvec3& amin, const glm::dvec3& amax, const glm::dvec3& bmin, const glm::dvec3& bmax) {
    return (amin.x < bmax.x - COLLISION_EPS && amax.x > bmin.x + COLLISION_EPS) &&
           (amin.y < bmax.y - COLLISION_EPS && amax.y > bmin.y + COLLISION_EPS) &&
           (amin.z < bmax.z - COLLISION_EPS && amax.z > bmin.z + COLLISION_EPS);
}

void Camera::updateVelocity(float deltaTime, World* world) {
    // Fix deltaTime spikes (resizing/moving the window)
    if (deltaTime > 0.05f) deltaTime = 0.05f;

    // Substep for stability at low FPS
    const float maxSubstep = 1.0f / 200.0f;
    int numSteps = static_cast<int>(ceil(deltaTime / maxSubstep));
    float subDelta = deltaTime / numSteps;

    for (int i = 0; i < numSteps; ++i) {
        stepVelocity(subDelta, world);
    }
}

void Camera::stepVelocity(float deltaTime, World* world) {
    if (jumpBuffered) {
        jumpBufferTimer -= deltaTime;
        if (jumpBufferTimer <= 0.0f) {
            jumpBuffered = false;
            jumpBufferTimer = 0.0f;
        }
    }
    if (!grounded) {
        coyoteTimer -= deltaTime;
        if (coyoteTimer < 0.0f) coyoteTimer = 0.0f;
    } else {
        coyoteTimer = coyoteTime;
    }

    velocity.y += gravity * deltaTime;
    glm::vec3 proposedPos = position;
    glm::vec3 horizMove = glm::vec3(velocity.x, 0.0f, velocity.z) * deltaTime;
    double feetY_current = position.y - eyeHeight;

    auto isBlockSolid = [&](uint8_t type) -> bool {
        if (type == 0) return false;
        const BlockDB::BlockInfo* info = BlockDB::getBlockInfo(type);
        if (!info) return true;
        return !(info->modelName == "cross" || info->modelName == "liquid" || info->modelName == "pebble");
    };

    auto collidesWithTop = [&](const glm::dvec3& aabbMin, const glm::dvec3& aabbMax, World* world, double& outBlockTop) -> bool {
        for (int blockX = static_cast<int>(std::floor(aabbMin.x)); blockX <= static_cast<int>(std::floor(aabbMax.x)); ++blockX) {
            for (int blockZ = static_cast<int>(std::floor(aabbMin.z)); blockZ <= static_cast<int>(std::floor(aabbMax.z)); ++blockZ) {
                int chunkX = (blockX >= 0) ? (blockX / Chunk::chunkWidth) : ((blockX - Chunk::chunkWidth + 1) / Chunk::chunkWidth);
                int chunkZ = (blockZ >= 0) ? (blockZ / Chunk::chunkDepth) : ((blockZ - Chunk::chunkDepth + 1) / Chunk::chunkDepth);
                Chunk* chunk = world->getChunk(chunkX, chunkZ);
                if (!chunk) continue;

                for (int blockY = std::max(0, static_cast<int>(std::floor(aabbMin.y)));
                     blockY <= std::min(Chunk::chunkHeight - 1, static_cast<int>(std::floor(aabbMax.y)));
                     ++blockY) {
                    int localX = blockX - chunkX * Chunk::chunkWidth;
                    int localY = blockY;
                    int localZ = blockZ - chunkZ * Chunk::chunkDepth;
                    if (localX < 0 || localX >= Chunk::chunkWidth ||
                        localY < 0 || localY >= Chunk::chunkHeight ||
                        localZ < 0 || localZ >= Chunk::chunkDepth) continue;

                    uint8_t type = chunk->blocks[localX][localY][localZ].type;
                    if (!isBlockSolid(type)) continue;

                    glm::vec3 bmin_f, bmax_f;
                    getBlockAABB(type, bmin_f, bmax_f);
                    glm::dvec3 bmin = glm::dvec3(bmin_f) + glm::dvec3(blockX, blockY, blockZ);
                    glm::dvec3 bmax = glm::dvec3(bmax_f) + glm::dvec3(blockX, blockY, blockZ);

                    if (aabbOverlapStrict(aabbMin, aabbMax, bmin, bmax)) {
                        outBlockTop = bmax.y;
                        return true;
                    }
                }
            }
        }
        return false;
    };

    auto tryMoveOrStep = [&](const glm::vec3& tryPos, glm::vec3& outPos) -> bool {
        double feetY = tryPos.y - eyeHeight;
        glm::dvec3 aabbMin(tryPos.x - playerRadius, feetY, tryPos.z - playerRadius);
        glm::dvec3 aabbMax(tryPos.x + playerRadius, feetY + playerHeight, tryPos.z + playerRadius);

        double blockTop = 0.0;
        if (!collidesWithTop(aabbMin, aabbMax, world, blockTop)) {
            outPos = tryPos;
            return true;
        }

        // step up check
        double stepDiff = blockTop - feetY_current;
        if (grounded && stepDiff > -0.01 - COLLISION_EPS && stepDiff <= stepHeight + COLLISION_EPS) {
            glm::vec3 steppedPos = tryPos;
            steppedPos.y = static_cast<float>(blockTop + eyeHeight);
            double steppedFeetY = steppedPos.y - eyeHeight;

            glm::dvec3 sMin(steppedPos.x - playerRadius, steppedFeetY, steppedPos.z - playerRadius);
            glm::dvec3 sMax(steppedPos.x + playerRadius, steppedFeetY + playerHeight, steppedPos.z + playerRadius);

            double dummyTop;
            if (!collidesWithTop(sMin, sMax, world, dummyTop)) {
                outPos = steppedPos;
                velocity.y = 0.0f;
                grounded = true;
                return true;
            }
        }

        return false; // blocked
    };

    if (world) {
        glm::vec3 stepPos;

        // full X+Z
        if (tryMoveOrStep(position + horizMove, stepPos)) {
            proposedPos = stepPos;
        }
        // X only
        else if (tryMoveOrStep(position + glm::vec3(horizMove.x, 0, 0), stepPos)) {
            proposedPos.x = stepPos.x;
            proposedPos.y = stepPos.y;
            velocity.z = 0.0f;
        }
        // Z only
        else if (tryMoveOrStep(position + glm::vec3(0, 0, horizMove.z), stepPos)) {
            proposedPos.z = stepPos.z;
            proposedPos.y = stepPos.y;
            velocity.x = 0.0f;
        } else {
            velocity.x = velocity.z = 0.0f;
        }

        // vertical movement
        proposedPos.y += velocity.y * deltaTime;
        double feetY = proposedPos.y - eyeHeight;
        glm::dvec3 aabbMin(proposedPos.x - playerRadius, feetY, proposedPos.z - playerRadius);
        glm::dvec3 aabbMax(proposedPos.x + playerRadius, feetY + playerHeight, proposedPos.z + playerRadius);

        grounded = false;
        double blockTop;
        if (collidesWithTop(aabbMin, aabbMax, world, blockTop)) {
            if (feetY <= blockTop + 0.01 + COLLISION_EPS &&
                feetY >= blockTop - stepHeight - COLLISION_EPS) {
                proposedPos.y = static_cast<float>(blockTop + eyeHeight + COLLISION_EPS);
                velocity.y = 0.0f;
                grounded = true;
            } else {
                proposedPos.y = position.y;
                velocity.y = 0.0f;
            }
        }
    } else {
        proposedPos += horizMove;
        proposedPos.y += velocity.y * deltaTime;
    }

    if (jumpBuffered && (grounded || coyoteTimer > 0.0f)) {
        velocity.y = jumpPower;
        grounded = false;
        jumpBuffered = false;
        jumpBufferTimer = 0.0f;
        coyoteTimer = 0.0f;
        proposedPos.y += 0.001f;
    }

    position = proposedPos;

    // drag
    glm::vec3 horizVel = glm::vec3(velocity.x, 0.0f, velocity.z);
    float drag = 9.0f;
    horizVel -= horizVel * glm::min(drag * deltaTime, 1.0f);
    if (glm::length(horizVel) < 0.01f) horizVel = glm::vec3(0.0f);
    velocity.x = horizVel.x;
    velocity.z = horizVel.z;
}

void Camera::updateVelocityFlight(float deltaTime) {
    position += velocity * deltaTime;

    float drag = 9.0f;
    velocity -= velocity * glm::min(drag * deltaTime, 1.0f);

    if (glm::length(velocity) < 0.01f)
        velocity = glm::vec3(0.0f);
}

void Camera::applyAcceleration(const glm::vec3& acceleration, float deltaTime) {
    //glm::vec3 horizAccel = glm::vec3(acceleration.x, 0.0f, acceleration.z);
    //velocity += horizAccel * deltaTime;
    velocity += acceleration * deltaTime;
}

void Camera::jump() {
    jumpBuffered = true;
    jumpBufferTimer = jumpBufferTime;

    if (grounded) {
        velocity.y = jumpPower;
        grounded = false;
        jumpBuffered = false;
        jumpBufferTimer = 0.0f;
        coyoteTimer = 0.0f;
    }
}

void Camera::setPosition(const glm::vec3& pos) {
    position = pos;
    velocity = glm::vec3(0.0f);
    grounded = false;
}
