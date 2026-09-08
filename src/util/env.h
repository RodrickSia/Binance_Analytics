#pragma once

#include <cstdlib>
#include <fstream>
#include <string>

namespace util {

// Loads KEY=VALUE pairs from a .env file into the process environment,
inline void load_dotenv(const std::string& path = ".env") {
    std::ifstream file(path);
    if (!file) {
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        const auto eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        const std::string key = line.substr(0, eq);
        const std::string value = line.substr(eq + 1);
        setenv(key.c_str(), value.c_str(), 0);
    }
}

} // namespace util
