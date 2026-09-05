#pragma once

#include <iostream>
#include <string>

namespace util {

inline void log_info(const std::string& message) {
    std::clog << "[info] " << message << std::endl;
}

inline void log_error(const std::string& message) {
    std::clog << "[error] " << message << std::endl;
}

inline void log_debug(const std::string& message) {
    std::clog << "[debug] " << message << std::endl;
}

} // namespace util
