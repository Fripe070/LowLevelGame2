#include "managers.h"

#include "loader/mesh.h"
#include "loader/texture.h"


namespace Engine::Manager {
    std::expected<unsigned int, std::string> TextureManager::loadTexture(const std::string &texturePath) {
        if (textures.contains(texturePath))
            return textures[texturePath];

        std::expected<unsigned int, std::string> texture = Loader::loadTexture(texturePath);
        if (!texture.has_value())
            return std::unexpected(texture.error());
        return textures[texturePath] = texture.value();
    }

}
