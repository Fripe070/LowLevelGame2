#include "managers.h"

#include <stdexcept>
#include <gl/glew.h>

#include "loader/mesh.h"
#include "loader/texture.h"


namespace Engine::Manager {
    TextureManager::TextureManager() {
        const auto errorTxt = Loader::loadTexture(ERROR_TEXTURE_PATH);
        if (!errorTxt.has_value())
            throw std::runtime_error("Failed to load error texture: " + errorTxt.error());
        errorTexture = errorTxt.value();
    }

    TextureManager::~TextureManager() {
        clear();
        glDeleteTextures(1, &errorTexture);
    }

    void TextureManager::clear() {
        for (const auto &[path, textureID] : textures)
            glDeleteTextures(1, &textureID);
        textures.clear();
    }

    std::expected<unsigned int, std::string> TextureManager::loadTexture(const std::string &texturePath) {
        if (textures.contains(texturePath))
            return textures[texturePath];

        std::expected<unsigned int, std::string> texture = Loader::loadTexture(texturePath);
        if (!texture.has_value()) {
            textures[texturePath] = errorTexture;
            return std::unexpected(texture.error());
        }
        return textures[texturePath] = texture.value();
    }

    bool TextureManager::unloadTexture(const std::string &texturePath) {
        if (!textures.contains(texturePath))
            return false;
        glDeleteTextures(1, &textures[texturePath]);
        textures.erase(texturePath);
        return true;
    }

}
