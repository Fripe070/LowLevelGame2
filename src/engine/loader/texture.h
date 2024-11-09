#ifndef TEXTURE_H
#define TEXTURE_H
#include <expected>
#include <string>

namespace Engine::Loader {
    std::expected<unsigned int, std::string> loadTexture(const char* filePath);
    std::expected<unsigned int, std::string> loadTexture(const std::string &filePath);
}

#endif //TEXTURE_H
