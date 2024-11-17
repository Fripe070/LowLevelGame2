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
        /*!
         * @brief Get the OpenGL texture ID associated with a texture path, loading it if necessary
         * @param texturePath The path to the texture
         * @return The texture ID or an error message
         */
        std::expected<unsigned int, std::string> loadTexture(const std::string &texturePath);
        bool unloadTexture(const std::string &texturePath);
    };

}

#endif
