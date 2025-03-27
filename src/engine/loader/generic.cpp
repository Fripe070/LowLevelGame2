#include "generic.h"

#include <fstream>

#include "engine/util/error.h"
#include "engine/util/logging.h"

std::expected<std::string, Error> Engine::readTextFile(const std::string &filePath) {
    std::ifstream file(filePath);
    if (!file.is_open())
        return std::unexpected(ERROR("Failed to open file: " + filePath));

    std::string fileContents((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());
    return fileContents;
}
