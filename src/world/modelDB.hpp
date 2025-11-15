#pragma once

#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <vector>

struct Face {
    std::vector<std::pair<float, float>> uv;
};

struct Cuboid {
    glm::vec3 from;
    glm::vec3 to;
    std::unordered_map<std::string, Face> faces;
    
    struct CollisionBox {
        glm::vec3 from;
        glm::vec3 to;
    };

    struct Collisions {
        bool enabled = true;
        std::vector<CollisionBox> box;
    } collisions;
};

struct Plane {
    glm::vec3 from;
    glm::vec3 to;
    glm::vec3 rotationOrigin{};
    char rotationAxis = '\0';
    float rotationAngle = 0.0f;
    char positionDirection = '\0';
    float positionOffset = 0.0f;
    std::unordered_map<std::string, Face> faces;

	struct CollisionBox {
        glm::vec3 from;
        glm::vec3 to;
    };

    struct Collisions {
        bool enabled = true;
        std::vector<CollisionBox> box;
    } collisions;
};

struct Model {
    std::vector<Cuboid> cuboids;
    std::vector<Plane> planes;
};

class ModelDB {
public:
    static void loadAllModels(const std::string& modelsDir);
    static const Model* getModel(const std::string& name);
    static bool getCollisionBoxes(const std::string& modelName, std::vector<std::pair<glm::vec3, glm::vec3>>& outBoxes);
    static bool getHitBoxes(const std::string& modelName, std::vector<std::pair<glm::vec3, glm::vec3>>& outBoxes);
    
private:
    static std::unordered_map<std::string, Model> models;
};
