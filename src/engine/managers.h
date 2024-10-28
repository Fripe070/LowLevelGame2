#ifndef MANAGERS_H
#define MANAGERS_H

#include <expected>
#include <string>
#include <unordered_map>

#define ERROR_TEXTURE_PATH "resources/error.png"

namespace Engine::Manager {

    // TODO: Clear in destructor?
    class TextureManager {
    private:
        std::unordered_map<std::string, unsigned int> textures;

    public:
        unsigned int errorTexture;

        TextureManager();
        ~TextureManager();

        void clear();
        std::expected<unsigned int, std::string> loadTexture(const std::string &texturePath);
        std::expected<unsigned int, std::string> loadTexture(const char *texturePath) {
            return loadTexture(std::string(texturePath));
        }
    };

}

#endif
