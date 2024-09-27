#ifndef UTILITY_H
#define UTILITY_H
#include <expected>
#include <string>


std::expected<std::string, std::string> readTextFile(const char* filePath);
std::expected<std::string, std::string> readTextFile(const std::string &filePath);

std::expected<unsigned int, std::string> loadTexture(const char* filePath);
std::expected<unsigned int, std::string> loadTexture(const std::string &filePath);

#endif //UTILITY_H
