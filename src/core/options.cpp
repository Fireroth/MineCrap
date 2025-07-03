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
