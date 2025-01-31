#include "engine/manager/texture.h"

#include <stdexcept>
#include <GL/glew.h>

#include "engine/loader/texture.h"

#include <engine/logging.h>


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

    std::expected<unsigned int, std::string> TextureManager::getTexture(const std::string &texturePath, const TextureType type) {
        if (texturePath.empty())
            return errorTexture;

        if (textures.contains(texturePath))
            return textures[texturePath];

#define BREAK_CASE(x, ...) case x: __VA_ARGS__; break;
        std::expected<unsigned int, std::string> texture;
        switch (type) {
            BREAK_CASE(TextureType::TEXTURE_2D, texture = Loader::loadTexture(texturePath.c_str()));
            BREAK_CASE(TextureType::CUBEMAP, texture = Loader::loadCubeMap(texturePath));
            default:
                return std::unexpected("Unknown texture type");
        }
#undef BREAK_CASE
        if (!texture.has_value()) {
            textures[texturePath] = errorTexture;  // Only error once, then use the error texture
            return std::unexpected(FW_UNEXP(texture, "Failed to load uncached texture"));
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
