#pragma once
#include <expected>
#include <string>
#include <engine/util/error.h>

namespace Engine
{
    std::expected<std::string, Error> readTextFile(const std::string &filePath);
}