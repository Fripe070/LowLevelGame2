#include "generic.h"
#include <fstream>

namespace Engine::Loader {
    std::expected<std::string, std::string> readTextFile(const std::string &filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            return std::unexpected("Failed to open file: " + filePath);
        }

        std::string fileContents((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());
        return fileContents;
    }
    std::expected<std::string, std::string> readTextFile(const char* filePath) {
        return readTextFile(std::string(filePath));
    }
}
