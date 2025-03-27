#include "resource_manager.h"

#include <numeric>

// Generated header files for embedded resources
#include <error_obj.h>
#include <error_png.h>
#include <error_shader_frag.h>
#include <error_shader_vert.h>


namespace Engine {
    ResourceManager::ResourceManager()
    {
        // TEXTURE
        const auto tmpTex = Loader::loadTexture(BIN_ERROR_PNG.data(), BIN_ERROR_PNG.size());
        if (!tmpTex.has_value())
            throw std::runtime_error(stringifyError(FW_ERROR(tmpTex.error(), "Failed to load error texture")));
        errorTexture = std::make_shared<ManagedTexture>(tmpTex.value());

        // SCENE
        std::expected<Loader::Scene, Error> tmpScene = Loader::loadScene(BIN_ERROR_OBJ.data(), BIN_ERROR_OBJ.size());
        if (!tmpScene.has_value())
            throw std::runtime_error(stringifyError(FW_ERROR(tmpScene.error(), "Failed to load error scene")));
        errorScene = std::make_shared<Loader::Scene>(std::move(tmpScene.value()));

        // SHADER
        const auto vertShader = Engine::loadShaderString(
            std::string(BIN_ERROR_SHADER_VERT.begin(), BIN_ERROR_SHADER_VERT.end()),
            ShaderType::VERTEX);
        if (!vertShader.has_value())
            throw std::runtime_error(stringifyError(FW_ERROR(vertShader.error(), "Failed to load error vertex shader")));

        const auto fragShader = Engine::loadShaderString(
            std::string(BIN_ERROR_SHADER_FRAG.begin(), BIN_ERROR_SHADER_FRAG.end()),
            ShaderType::FRAGMENT);
        if (!fragShader.has_value())
            throw std::runtime_error(stringifyError(FW_ERROR(fragShader.error(), "Failed to load error fragment shader")));

        errorShader = std::make_shared<ShaderProgram>(std::vector{
            vertShader.value(),
            fragShader.value()
        });
    }

    std::shared_ptr<Loader::Scene>
    ResourceManager::loadScene(const std::string& scenePath)
    {
        if (scenes.contains(scenePath)) {
            if (scenes[scenePath].expired())
                scenes.erase(scenePath);
            else
                return scenes[scenePath].lock();
        }

        std::expected<Loader::Scene, Error> scene = Loader::loadScene(scenePath);
        if (!scene.has_value()) {
            scenes[scenePath] = errorScene;  // Only error once, then use the error scene
            reportError(FW_ERROR(scene.error(), "Failed to load uncached scene"));
            return errorScene;
        }
        auto ptr = std::make_shared<Loader::Scene>(std::move(scene.value()));
        scenes[scenePath] = ptr;
        return ptr;
    }

    std::shared_ptr<ManagedTexture>
    ResourceManager::loadTexture(const std::string& texturePath, const TextureType type)
    {
        if (textures.contains(texturePath)) {
            if (textures[texturePath].expired())
                textures.erase(texturePath);
            else
                return textures[texturePath].lock();
        }

        std::expected<unsigned int, Error> textureID;
        switch (type) {
        case TextureType::TEXTURE_2D: textureID = Loader::loadTexture(texturePath.c_str()); break;
        case TextureType::CUBEMAP:    textureID = Loader::loadCubeMap(texturePath); break;
        default:
            reportError(ERROR(fmt::format("Unknown texture type: {}", static_cast<int>(type))));
            return errorTexture;
        }
        if (!textureID.has_value()) {
            textures[texturePath] = errorTexture;  // Only error once, then use the error texture
            reportError(FW_ERROR(textureID.error(), "Failed to load uncached texture"));
            return errorTexture;
        }

        auto ptr = std::make_shared<ManagedTexture>(textureID.value());
        textures[texturePath] = ptr;
        return ptr;
    }

    std::shared_ptr<ShaderProgram>
    ResourceManager::loadShader(const std::vector<std::pair<std::string, ShaderType>>& shaders)
    {
        std::string jointPath; // Used as an identifier for this specific combo of shaders
        for (const auto& [path, type] : shaders)
            jointPath += path;

        if (this->shaders.contains(jointPath)) {
            if (this->shaders[jointPath].expired())
                this->shaders.erase(jointPath);
            else
                return this->shaders[jointPath].lock();
        }

        std::vector<unsigned int> shaderIDs;
        shaderIDs.reserve(shaders.size());
        for (const auto& [path, type] : shaders) {
            std::expected<unsigned int, Error> shaderID = loadShaderFile(path, type);
            if (!shaderID.has_value()) {
                this->shaders[jointPath] = errorShader;  // Only error once, then use the error shader
                reportError(FW_ERROR(shaderID.error(), "Failed to load uncached shader"));
                return errorShader;
            }
            shaderIDs.push_back(shaderID.value());
        }

        auto ptr = std::make_shared<ShaderProgram>(shaderIDs);
        this->shaders[jointPath] = ptr;
        return ptr;
    }


    std::shared_ptr<ShaderProgram> ResourceManager::loadShader(std::string computePath)
    { return loadShader({{computePath, ShaderType::COMPUTE} }); }

    std::shared_ptr<ShaderProgram> ResourceManager::loadShader(std::string vertexPath, std::string fragmentPath)
    { return loadShader({
        {vertexPath, ShaderType::VERTEX},
        {fragmentPath, ShaderType::FRAGMENT}}); }

    std::shared_ptr<ShaderProgram> ResourceManager::loadShader(std::string vertexPath, std::string geometryPath, std::string fragmentPath)
    { return loadShader({
        {vertexPath, ShaderType::VERTEX},
        {geometryPath, ShaderType::GEOMETRY},
        {fragmentPath, ShaderType::FRAGMENT}}); }



}
