#ifndef MANAGER_MESH_H
#define MANAGER_MESH_H

#include <engine/loader/scene.h>
#include <expected>
#include <memory>
#include <string>
#include <unordered_map>

#define ERROR_MESH_PATH "resources/assets/models/error.obj"

struct aiNode;

namespace Engine::Manager {
    // TODO: Hot reloading
    // TODO: Maybe a tad *too* ready to unload unused assets?
    class SceneManager {
    private:
        std::unordered_map<std::string, std::weak_ptr<Loader::Scene>> scenes;
    public:
        std::shared_ptr<Loader::Scene> errorScene;

        SceneManager();

        std::expected<std::shared_ptr<Loader::Scene>, std::string> getScene(const std::string &scenePath);
    };

}

#endif
