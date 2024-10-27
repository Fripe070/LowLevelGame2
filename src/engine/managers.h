#ifndef MANAGERS_H
#define MANAGERS_H

#include <expected>
#include <string>
#include <unordered_map>

namespace Engine::Manager {

    // TODO: Clear in destructor?
    class TextureManager {
    public:
        std::unordered_map<std::string, unsigned int> textures;
        std::expected<unsigned int, std::string> loadTexture(const std::string &texturePath);
    };

}

#endif
