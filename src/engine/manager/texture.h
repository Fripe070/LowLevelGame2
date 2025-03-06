#ifndef MANAGER_TEXTURE_H
#define MANAGER_TEXTURE_H

#include <expected>
#include <string>
#include <unordered_map>

#define ERROR_TEXTURE_PATH "resources/assets/textures/error.png"

namespace Engine {
    enum class TextureType {
        TEXTURE_2D,
        CUBEMAP
    };
}

namespace Engine::Manager {
    /*!
     * Class that stores and manages OpenGL texture IDs to avoid loading the same texture multiple times
     */
    class TextureManager {
    private:
        std::unordered_map<std::string, unsigned int> textures;
        // TODO: Hot reloading
        // TODO: Implement a reference counting system like we have for scenees

    public:
        unsigned int errorTexture;

        TextureManager();
        ~TextureManager();

        /*!
         * @brief Get the OpenGL texture ID associated with a texture path, loading it if necessary
         * @param texturePath The path to the texture
         * @param type The type of texture to load. Defaults to a 2D texture
         * @return The texture ID or an error message
         */
        std::expected<unsigned int, std::string> getTexture(const std::string &texturePath, TextureType type = TextureType::TEXTURE_2D);

        /*!
         * @brief Unload a texture
         * @param texturePath The path to the texture
         * @return Whether the texture was successfully unloaded
         */
        bool unloadTexture(const std::string &texturePath);

        /*!
         * @brief Unload all textures (except the error texture)
         */
        void clear();
    };
}

#endif
