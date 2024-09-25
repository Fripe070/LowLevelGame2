#ifndef UTILITY_H
#define UTILITY_H
#include <expected>


std::expected<std::string, std::string> readTextFile(const std::string &filePath);

#endif //UTILITY_H
