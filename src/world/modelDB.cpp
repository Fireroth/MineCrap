#include "modelDB.hpp"
#include <filesystem>
#include <fstream>
#include <cstdio>
#include <nlohmannJSON/json.hpp>

using json = nlohmann::json;
namespace fs = std::filesystem;

std::unordered_map<std::string, Model> ModelDB::models;

void ModelDB::loadAllModels(const std::string& modelsDir) {
    models.clear();
    for (const auto& entry : fs::directory_iterator(modelsDir)) {
        if (!entry.is_regular_file())
            continue;
        auto path = entry.path();
        if (path.extension() != ".json")
            continue;
        std::ifstream file(path);
        if (!file)
            continue;
        json j;
        try {
            file >> j;
        } catch (...) {
            continue;
        }
        Model model;
        if (j.contains("cuboids")) {
            for (const auto& cuboidJ : j["cuboids"]) {
                Cuboid cuboid;
                auto from = cuboidJ["from"];
                auto to = cuboidJ["to"];
                cuboid.from = glm::vec3(from[0], from[1], from[2]);
                cuboid.to = glm::vec3(to[0], to[1], to[2]);
                if (cuboidJ.contains("faces")) {
                    for (auto& [faceName, faceJ] : cuboidJ["faces"].items()) {
                        Face face;
                        for (const auto& uv : faceJ["uv"]) {
                            face.uv.emplace_back(uv[0], uv[1]);
                        }
                        cuboid.faces[faceName] = face;
                    }
                }
                // parse collisions if present
                if (cuboidJ.contains("collisions")) {
                    const auto &colJ = cuboidJ["collisions"];
                    if (colJ.contains("enabled")) cuboid.collisions.enabled = colJ["enabled"].get<bool>();
                    if (colJ.contains("box") && colJ["box"].is_array()) {
                        for (const auto &boxJ : colJ["box"]) {
                            Cuboid::CollisionBox cb;
                            auto bf = boxJ["from"];
                            auto bt = boxJ["to"];
                            cb.from = glm::vec3(bf[0], bf[1], bf[2]);
                            cb.to = glm::vec3(bt[0], bt[1], bt[2]);
                            cuboid.collisions.box.push_back(cb);
                        }
                    }
                }
                model.cuboids.push_back(cuboid);
            }
        }

        if (j.contains("plane") || j.contains("planes")) {
            const auto &planesJ = j.contains("planes") ? j["planes"] : j["plane"];
            for (const auto& planeJ : planesJ) {
                Plane plane;
                // from/to are required
                auto from = planeJ["from"];
                auto to = planeJ["to"];
                plane.from = glm::vec3(from[0], from[1], from[2]);
                plane.to = glm::vec3(to[0], to[1], to[2]);
                if (planeJ.contains("rotation")) {
                    const auto &r = planeJ["rotation"];
                    if (r.contains("origin")) {
                        auto o = r["origin"];
                        plane.rotationOrigin = glm::vec3(o[0], o[1], o[2]);
                    }
                    if (r.contains("axis")) {
                        std::string a = r["axis"].get<std::string>();
                        if (!a.empty()) plane.rotationAxis = a[0];
                    }
                    if (r.contains("angle")) {
                        plane.rotationAngle = r["angle"].get<float>();
                    }
                    if (r.contains("position") && r["position"].is_number()) {
                        plane.positionOffset = r["position"].get<float>();
                        if (plane.rotationAxis != '\0' && plane.positionDirection == '\0') {
                            plane.positionDirection = plane.rotationAxis;
                        }
                    }
                }

                if (planeJ.contains("position")) {
                    const auto &pp = planeJ["position"];
                    if (pp.is_number()) {
                        plane.positionOffset = pp.get<float>();
                        if (plane.rotationAxis != '\0' && plane.positionDirection == '\0') {
                            plane.positionDirection = plane.rotationAxis;
                        }
                    } else if (pp.is_object()) {
                        if (pp.contains("direction")) {
                            std::string d = pp["direction"].get<std::string>();
                            if (!d.empty()) plane.positionDirection = d[0];
                        }
                        if (pp.contains("offset")) {
                            plane.positionOffset = pp["offset"].get<float>();
                        }
                    }
                }
                if (planeJ.contains("faces")) {
                    for (auto& [faceName, faceJ] : planeJ["faces"].items()) {
                        Face face;
                        if (faceJ.contains("uv")) {
                            for (const auto& uv : faceJ["uv"]) {
                                face.uv.emplace_back(uv[0], uv[1]);
                            }
                        }
                        plane.faces[faceName] = face;
                    }
                }
                if (planeJ.contains("collisions")) {
                    const auto &colJ = planeJ["collisions"];
                    if (colJ.contains("enabled")) plane.collisions.enabled = colJ["enabled"].get<bool>();
                    if (colJ.contains("box") && colJ["box"].is_array()) {
                        for (const auto &boxJ : colJ["box"]) {
                            Plane::CollisionBox cb;
                            auto bf = boxJ["from"];
                            auto bt = boxJ["to"];
                            cb.from = glm::vec3(bf[0], bf[1], bf[2]);
                            cb.to = glm::vec3(bt[0], bt[1], bt[2]);
                            plane.collisions.box.push_back(cb);
                        }
                    }
                }
                model.planes.push_back(plane);
            }
        }
        std::string name = path.stem().string();
        models[name] = std::move(model);
    }
}

const Model* ModelDB::getModel(const std::string& name) {
    auto it = models.find(name);
    if (it != models.end()) return &it->second;
    return nullptr;
}

bool ModelDB::getCollisionBoxes(const std::string& modelName, std::vector<std::pair<glm::vec3, glm::vec3>>& outBoxes) {
    auto it = models.find(modelName);
    if (it == models.end()) return false;
    const Model& model = it->second;

    for (const auto& cub : model.cuboids) {
        if (!cub.collisions.enabled) continue;

        if (!cub.collisions.box.empty()) {
            for (const auto& b : cub.collisions.box) {
                outBoxes.emplace_back(b.from, b.to);
            }
        } else {
            // use cuboid from/to as fallback
            outBoxes.emplace_back(cub.from, cub.to);
        }
    }

    for (const auto& plane : model.planes) {
        if (!plane.collisions.enabled) continue;

        if (!plane.collisions.box.empty()) {
            for (const auto& b : plane.collisions.box) {
                outBoxes.emplace_back(b.from, b.to);
            }
        } else {
            // use plane from/to as fallback
            outBoxes.emplace_back(plane.from, plane.to);
        }
    }

    return !outBoxes.empty();
}

bool ModelDB::getHitBoxes(const std::string& modelName, std::vector<std::pair<glm::vec3, glm::vec3>>& outBoxes) {
    auto it = models.find(modelName);
    if (it == models.end()) return false;
    const Model& model = it->second;

    for (const auto& cub : model.cuboids) {
        if (!cub.collisions.box.empty()) {
            for (const auto& b : cub.collisions.box) {
                outBoxes.emplace_back(b.from, b.to);
            }
        } else {
            // use cuboid from/to as fallback
            outBoxes.emplace_back(cub.from, cub.to);
        }
    }
    
    if (!model.planes.empty()) {
        const auto &plane = model.planes.front();
        if (!plane.collisions.box.empty()) {
            for (const auto& b : plane.collisions.box) {
                outBoxes.emplace_back(b.from, b.to);
            }
        } else {
            // use plane from/to as fallback
            outBoxes.emplace_back(plane.from, plane.to);
        }
    }

    return !outBoxes.empty();
}

