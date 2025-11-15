#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmannJSON/json.hpp>
#include "blockDB.hpp"
#include "../core/options.hpp"

std::unordered_map<uint8_t, BlockDB::BlockInfo> BlockDB::blockData;

void BlockDB::init() {
    blockData.clear();

    namespace fs = std::filesystem;
    fs::path blocksDir = fs::current_path() / "blocks";

    if (!fs::exists(blocksDir) || !fs::is_directory(blocksDir)) {
        std::cerr << "BlockDB::init: could not find 'blocks' directory at "
                  << blocksDir << std::endl;
        return;
    }

    // Temp entries to pick fast/slow variant
    struct TempEntry {
        std::string baseName;
        std::string variant;
        int id;
        BlockInfo info;
    };

    std::vector<TempEntry> tempEntries;

    for (auto &entry : fs::directory_iterator(blocksDir)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".json")
            continue;

        std::ifstream in(entry.path());
        if (!in.is_open()) {
            std::cerr << "BlockDB::init: failed to open " << entry.path() << std::endl;
            continue;
        }

        try {
            nlohmann::json j;
            in >> j;

            for (auto it = j.begin(); it != j.end(); it++) {
                const std::string blockKey = it.key();
                const auto &obj = it.value();

                if (!obj.contains("id")) continue;
                int id = obj["id"].get<int>();
                if (id < 0 || id > 255) continue;

                BlockInfo info;
                for (int t = 0; t < 6; t++)
                    info.textureCoords[t] = glm::vec2(0.0f);

                info.multiTextureCoords.clear();
                for (int setIndex = 1; setIndex <= 16; setIndex++) {
                    std::string key = (setIndex == 1) ? std::string("textures") : std::string("textures") + std::to_string(setIndex);
                    if (!obj.contains(key) || !obj[key].is_array()) break;

                    std::array<glm::vec2,6> setArr;
                    for (int i = 0; i < 6; i++) setArr[i] = glm::vec2(0.0f);

                    int idx = 0;
                    for (const auto &tex : obj[key]) {
                        if (idx >= 6) break;
                        if (tex.is_array() && tex.size() >= 2) {
                            setArr[idx] = glm::vec2(tex[0], tex[1]);
                        } else if (tex.is_number()) {
                            setArr[idx] = glm::vec2(tex.get<float>(), 0.0f);
                        }
                        idx++;
                    }
                    info.multiTextureCoords.push_back(setArr);
                }

                // Keep backward compatible with single textureCoords
                for (int t = 0; t < 6; t++)
                    info.textureCoords[t] = glm::vec2(0.0f);
                if (!info.multiTextureCoords.empty()) {
                    auto &first = info.multiTextureCoords.front();
                    for (int t = 0; t < 6; t++)
                        info.textureCoords[t] = first[t];
                }

                info.transparent = obj.value("transparent", false);
                info.liquid = obj.value("liquid", false);
                info.name = obj.value("name", blockKey);
                info.modelName = obj.value("model", std::string("cube"));
                info.renderFacesInBetween = obj.value("renderFacesInBetween", false);
                info.tabName = obj.value("tabName", std::string("Blocks"));

                std::string baseName = blockKey;
                std::string variant;
                if (auto pos = blockKey.find(':'); pos != std::string::npos) {
                    baseName = blockKey.substr(0, pos);
                    variant = blockKey.substr(pos + 1);
                }

                tempEntries.push_back({baseName, variant, id, info});
            }

        } catch (std::exception &e) {
            std::cerr << "BlockDB::init: JSON parse error in "
                      << entry.path() << ": " << e.what() << std::endl;
            continue;
        }
    }

    bool fasterTrees = (getOptionInt("faster_trees", 0) != 0);
    std::string preferred = fasterTrees ? "fast" : "slow";
    std::string fallback  = fasterTrees ? "slow" : "fast";

    std::unordered_map<std::string, std::vector<TempEntry>> grouped;
    for (auto &te : tempEntries)
        grouped[te.baseName].push_back(te);

    for (auto &[baseName, vec] : grouped) {
        TempEntry *chosen = nullptr;

        for (auto &te : vec)
            if (te.variant == preferred)
                chosen = &te;

        if (!chosen)
            for (auto &te : vec)
                if (te.variant == fallback)
                    chosen = &te;

        if (!chosen)
            chosen = &vec[0];

        blockData[(uint8_t)chosen->id] = chosen->info;
    }
}

const BlockDB::BlockInfo* BlockDB::getBlockInfo(const uint8_t& blockName) {
    auto iterator = blockData.find(blockName);
    if (iterator != blockData.end()) {
        return &iterator->second;
    }
    return nullptr;
}
