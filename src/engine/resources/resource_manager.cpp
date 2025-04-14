#include "resource_manager.h"

#include <numeric>

// Generated header files for embedded resources
#include <error_obj.h>
#include <error_png.h>
#include <error_shader_frag.h>
#include <error_shader_vert.h>


namespace Engine {
    ResourceManager::ResourceManager() {
        // Init error resources to an almost valid state
        errorShader = std::make_shared<Resource::Shader>(0);
        errorTexture = std::make_shared<Resource::ManagedTexture>(0);
        errorScene = std::make_shared<Resource::Scene>();
    }

    Expected<void> ResourceManager::populateErrorResources()
    {
        // TEXTURE
        const auto tmpTex = Resource::Loading::loadTexture(BIN_ERROR_PNG.data(), BIN_ERROR_PNG.size());
        if (!tmpTex.has_value())
            return std::unexpected(FW_ERROR(tmpTex.error(), "Failed to load error texture"));
        errorTexture = std::make_shared<Resource::ManagedTexture>(tmpTex.value());

        // SHADER
        const auto vertShaderID = Resource::Loading::loadGLShaderSource(
            std::string(BIN_ERROR_SHADER_VERT.begin(), BIN_ERROR_SHADER_VERT.end()),
            Resource::ShaderType::VERTEX
        );
        if (!vertShaderID.has_value())
            return std::unexpected(FW_ERROR(vertShaderID.error(), "Failed to load error vertex shader"));
        const auto fragShaderID = Resource::Loading::loadGLShaderSource(
            std::string(BIN_ERROR_SHADER_FRAG.begin(), BIN_ERROR_SHADER_FRAG.end()),
            Resource::ShaderType::FRAGMENT
        );
        if (!fragShaderID.has_value())
            return std::unexpected(FW_ERROR(fragShaderID.error(), "Failed to load error fragment shader"));
        errorShader = std::make_shared<Resource::Shader>(std::vector{
            vertShaderID.value(),
            fragShaderID.value()
        });

        // SCENE
        std::expected<Resource::Scene, Error> tmpScene = Resource::Loading::loadScene(BIN_ERROR_OBJ.data(), BIN_ERROR_OBJ.size());
        if (!tmpScene.has_value())
            return std::unexpected(FW_ERROR(tmpScene.error(), "Failed to load error scene"));
        errorScene = std::make_shared<Resource::Scene>(std::move(tmpScene.value()));

        return {};
    }

    std::shared_ptr<Resource::Scene>
    ResourceManager::loadScene(const std::string& scenePath)
    {
        if (scenes.contains(scenePath)) {
            if (scenes[scenePath].expired())
                scenes.erase(scenePath);
            else
                return scenes[scenePath].lock();
        }

        std::expected<Resource::Scene, Error> scene = Resource::Loading::loadScene(scenePath);
        if (!scene.has_value()) {
            scenes[scenePath] = errorScene;  // Only error once, then use the error scene
            reportError(FW_ERROR(scene.error(), "Failed to load uncached scene"));
            return errorScene;
        }
        auto ptr = std::make_shared<Resource::Scene>(std::move(scene.value()));
        scenes[scenePath] = ptr;
        return ptr;
    }

    std::shared_ptr<Resource::ManagedTexture>
    ResourceManager::loadTexture(const std::string& texturePath, const Resource::TextureType type)
    {
        if (textures.contains(texturePath)) {
            if (textures[texturePath].expired())
                textures.erase(texturePath);
            else
                return textures[texturePath].lock();
        }

        std::expected<unsigned int, Error> textureID;
        switch (type) {
            case Resource::TextureType::TEXTURE_2D: textureID = Resource::Loading::loadTexture(texturePath.c_str()); break;
            case Resource::TextureType::CUBEMAP:    textureID = Resource::Loading::loadCubeMap(texturePath); break;
            default:
                reportError(ERROR(fmt::format("Unknown texture type: {}", static_cast<int>(type))));
                return errorTexture;
        }
        if (!textureID.has_value()) {
            textures[texturePath] = errorTexture;  // Only error once, then use the error texture
            reportError(FW_ERROR(textureID.error(), "Failed to load uncached texture"));
            return errorTexture;
        }

        auto ptr = std::make_shared<Resource::ManagedTexture>(textureID.value());
        textures[texturePath] = ptr;
        return ptr;
    }

    std::shared_ptr<Resource::Shader>
    ResourceManager::loadShader(const std::map<Resource::ShaderType, std::string>& shaders)
    {
        std::string jointPath; // Used as an identifier for this specific combo of shaders
        for (const auto& [type, path] : shaders)
            jointPath += path + std::to_string(type);

        if (this->shaders.contains(jointPath)) {
            if (this->shaders[jointPath].expired())
                this->shaders.erase(jointPath);
            else
                return this->shaders[jointPath].lock();
        }

        std::vector<unsigned int> shaderIDs;
        shaderIDs.reserve(shaders.size());
        for (const auto& [type, path] : shaders) {
            std::expected<unsigned int, Error> shaderID = Resource::Loading::loadGLShaderFile(path, type);
            if (!shaderID.has_value()) {
                this->shaders[jointPath] = errorShader;  // Only error once, then use the error shader
                reportError(FW_ERROR(shaderID.error(), "Failed to load uncached shader"));
                return errorShader;
            }
            shaderIDs.push_back(shaderID.value());
        }

        auto ptr = std::make_shared<Resource::Shader>(shaderIDs);
        this->shaders[jointPath] = ptr;
        return ptr;
    }


    std::shared_ptr<Resource::Shader> ResourceManager::loadShader(std::string computePath)
    { return loadShader({
        {Resource::ShaderType::COMPUTE, computePath} }); }

    std::shared_ptr<Resource::Shader> ResourceManager::loadShader(std::string vertexPath, std::string fragmentPath)
    { return loadShader({
        {Resource::ShaderType::VERTEX, vertexPath},
        {Resource::ShaderType::FRAGMENT, fragmentPath}}); }

    std::shared_ptr<Resource::Shader> ResourceManager::loadShader(std::string vertexPath, std::string geometryPath, std::string fragmentPath)
    { return loadShader({
        {Resource::ShaderType::VERTEX, vertexPath},
        {Resource::ShaderType::GEOMETRY, geometryPath},
        {Resource::ShaderType::FRAGMENT, fragmentPath}}); }
}
