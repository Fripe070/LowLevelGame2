#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "engine/resources/scene.h"
#include "engine/resources/shader.h"
#include "engine/resources/texture.h"

namespace Engine {
    class ResourceManager {
        // TODO: Hot reloading
    private:
        std::unordered_map<std::string, std::weak_ptr<Resource::Shader>> shaders;
        std::unordered_map<std::string, std::weak_ptr<Resource::ManagedTexture>> textures;
        std::unordered_map<std::string, std::weak_ptr<Resource::Scene>> scenes;

        std::shared_ptr<Resource::Shader> errorShader;
        std::shared_ptr<Resource::ManagedTexture> errorTexture;
        std::shared_ptr<Resource::Scene> errorScene;
    public:
        ResourceManager();

        // Shader
        [[nodiscard]] std::shared_ptr<Resource::Shader>
        loadShader(const std::vector<std::pair<std::string, Resource::ShaderType>>& shaders);
        [[nodiscard]] std::shared_ptr<Resource::Shader>
        loadShader(std::string computePath);
        [[nodiscard]] std::shared_ptr<Resource::Shader>
        loadShader(std::string vertexPath, std::string fragmentPath);
        [[nodiscard]] std::shared_ptr<Resource::Shader>
        loadShader(std::string vertexPath, std::string geometryPath, std::string fragmentPath);

        // Texture
        [[nodiscard]] std::shared_ptr<Resource::ManagedTexture>
        loadTexture(const std::string &texturePath, Resource::TextureType type = Resource::TextureType::TEXTURE_2D);

        // Scene
        [[nodiscard]] std::shared_ptr<Resource::Scene>
        loadScene(const std::string &scenePath);

    };
}
