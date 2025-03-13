#ifndef GENERIC_H
#define GENERIC_H
#include <expected>
#include <string>

namespace Engine::Loader {
    std::expected<std::string, std::string> readTextFile(const char* filePath);
    std::expected<std::string, std::string> readTextFile(const std::string &filePath);
}

#endif
