#pragma once
#include <expected>
#include <string>

#include "engine/util/error.h"

std::expected<std::string, Error> readTextFile(const std::string &filePath);
