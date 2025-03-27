#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <engine/loader/scene.h>
#include <engine/loader/texture.h>
#include <engine/loader/shader/shader.h>

namespace Engine {
    class ResourceManager {
        // TODO: Hot reloading
    private:
        std::unordered_map<std::string, std::weak_ptr<ShaderProgram>> shaders;
        std::unordered_map<std::string, std::weak_ptr<ManagedTexture>> textures;
        std::unordered_map<std::string, std::weak_ptr<Loader::Scene>> scenes;

        std::shared_ptr<ShaderProgram> errorShader;
        std::shared_ptr<ManagedTexture> errorTexture;
        std::shared_ptr<Loader::Scene> errorScene;
    public:
        ResourceManager();

        [[nodiscard]] std::shared_ptr<ShaderProgram>
        loadShader(const std::vector<std::pair<std::string, ShaderType>>& shaders);

        [[nodiscard]] std::shared_ptr<ManagedTexture>
        loadTexture(const std::string &texturePath, TextureType type = TextureType::TEXTURE_2D);

        [[nodiscard]] std::shared_ptr<Loader::Scene>
        loadScene(const std::string &scenePath);

        // Convenience functions

        [[nodiscard]] std::shared_ptr<ShaderProgram>
        loadShader(std::string computePath);

        [[nodiscard]] std::shared_ptr<ShaderProgram>
        loadShader(std::string vertexPath, std::string fragmentPath);

        [[nodiscard]] std::shared_ptr<ShaderProgram>
        loadShader(std::string vertexPath, std::string geometryPath, std::string fragmentPath);
    };
}
