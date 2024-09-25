#include <fstream>
#include <string>
#include <expected>


std::expected<std::string, std::string> readTextFile(const std::string &filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return std::string("Failed to open file: ") + filePath;
    }

    std::string fileContents((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());
    return fileContents;
}
