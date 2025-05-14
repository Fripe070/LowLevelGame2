#pragma once
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

#include "engine/resources/scene.h"
#include "engine/resources/shader.h"
#include "engine/resources/texture.h"

namespace Engine {
    class ResourceManager {
        // TODO: Hot reloading
    private:
        std::unordered_map<std::string, std::weak_ptr<Resource::Shader>> shaders{};
        std::unordered_map<std::string, std::weak_ptr<Resource::ManagedTexture>> textures{};
        std::unordered_map<std::string, std::weak_ptr<Resource::Scene>> scenes{};
    public:
        std::shared_ptr<Resource::Shader> errorShader;
        std::shared_ptr<Resource::ManagedTexture> errorTexture;
        std::shared_ptr<Resource::ManagedTexture> errorCubemap;
        std::shared_ptr<Resource::Scene> errorScene;

        /*!
         * @brief Creates a resource manager.
         * @note The constructor  will not load any resources and the manager will be in a largely INVALID state.
         *       You must call \ref populateErrorResources() "populateErrorResources()" to load the error resources.
         */
        ResourceManager();
        /*!
         * @brief Loads the error resources into the resource manager.
         * @note This can not be called before the constructor, as it requires an already constructed resource manager to construct some resources.
         */
        [[nodiscard]] Expected<void> populateErrorResources();

    public:
        // Shader
        [[nodiscard]] std::shared_ptr<Resource::Shader>
        loadShader(const std::map<Resource::ShaderType, std::string>& shaders);
        [[nodiscard]] std::shared_ptr<Resource::Shader>
        loadShader(std::string computePath);
        [[nodiscard]] std::shared_ptr<Resource::Shader>
        loadShader(std::string vertexPath, std::string fragmentPath);
        [[nodiscard]] std::shared_ptr<Resource::Shader>
        loadShader(std::string vertexPath, std::string geometryPath, std::string fragmentPath);

        // Texture
        [[nodiscard]] std::shared_ptr<Resource::ManagedTexture>
        loadTexture(const std::string &texturePath);
        [[nodiscard]] std::shared_ptr<Resource::ManagedTexture>
        loadCubemap(const std::string &cubemapPath);

        // Scene
        [[nodiscard]] std::shared_ptr<Resource::Scene>
        loadScene(const std::string &scenePath);
    };
}
