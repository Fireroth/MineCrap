#pragma once

#include <string>

void loadOptionsFromFile(const std::string& filename);
void saveOption(const std::string& key, int value, const std::string& filename);
int getOptionInt(const std::string& key, int defaultValue);
float getOptionFloat(const std::string& key, float defaultValue);