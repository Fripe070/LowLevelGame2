#include "generic.h"
#include <fstream>
#include <engine/logging.h>

namespace Engine::Loader {
    std::expected<std::string, std::string> readTextFile(const std::string &filePath) {
        std::ifstream file(filePath);
        if (!file.is_open())
            return UNEXPECTED_REF("Failed to open file: " + filePath);

        std::string fileContents((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());
        return fileContents;
    }
    std::expected<std::string, std::string> readTextFile(const char* filePath) {
        std::ifstream file(filePath);
        if (!file.is_open())
            return UNEXPECTED_REF("Failed to open file: " + std::string(filePath));

        std::string fileContents((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());
        return fileContents;
    }
}
