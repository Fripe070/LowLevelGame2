#include "engine/manager/scene.h"

#include <engine/logging.h>


namespace Engine::Manager {
    SceneManager::SceneManager()
    // This is so cursed...
    : errorScene([] {
        std::expected<Loader::Scene, std::string> errorScn = Loader::loadScene(ERROR_MESH_PATH);
        if (!errorScn.has_value())
            throw std::runtime_error(FW_UNEXP(errorScn, "Failed to load error model"));
        return std::make_shared<Loader::Scene>(std::move(errorScn.value()));
    }()) {}

    std::expected<SharedScene, std::string> SceneManager::getScene(const std::string &scenePath) {
        if (scenes.contains(scenePath))
            return scenes[scenePath];

        std::expected<Loader::Scene, std::string> scene = Loader::loadScene(scenePath);
        if (!scene.has_value()) {
            scenes[scenePath] = errorScene;  // Only error once, then use the error model
            return std::unexpected(FW_UNEXP(scene, "Failed to load uncached model"));
        }
        return scenes[scenePath] = std::make_shared<Loader::Scene>(std::move(scene.value()));
    }

    bool SceneManager::unloadScene(const std::string &scenePath) {
        if (!scenes.contains(scenePath))
            return false;
        scenes.erase(scenePath);
        return true;
    }

    void SceneManager::clear() {
        scenes.clear();
    }
}