#include <fstream>
#include <sstream>
#include <map>
#include <iostream>
#include "options.hpp"

static std::map<std::string, int> optionsMap;
static bool loaded = false;

void loadOptionsFromFile(const std::string& filename) {
    if (loaded) return;
    std::ifstream file(filename);
    if (!file.is_open()) return;
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key;
        int value;
        if (std::getline(iss, key, '=') && iss >> value) {
            optionsMap[key] = value;
        }
    }
    loaded = true;
}

void saveOption(const std::string& key, int value, const std::string& filename) {
    optionsMap[key] = value;

    std::ofstream file(filename, std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << filename << " for writing." << std::endl;
        return;
    }

    for (const auto& [key, value] : optionsMap) {
        file << key << "=" << value << "\n";
    }

    loaded = false;
    loadOptionsFromFile(filename);
}

int getOptionInt(const std::string& key, const int defaultValue) {
    if (!loaded) loadOptionsFromFile("options.txt");
    auto iterator = optionsMap.find(key);
    if (iterator != optionsMap.end()) return iterator->second;
    else std::cout << "Option '" << key << "' not found, returning default value: " << defaultValue << std::endl;
    return defaultValue;
}

float getOptionFloat(const std::string& key, const float defaultValue) {
    if (!loaded) loadOptionsFromFile("options.txt");
    auto iterator = optionsMap.find(key);
    if (iterator != optionsMap.end()) return static_cast<float>(iterator->second);
    else std::cout << "Option '" << key << "' not found, returning default value: " << defaultValue << std::endl;
    return defaultValue;
}
