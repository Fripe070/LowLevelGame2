#ifndef MANAGER_MESH_H
#define MANAGER_MESH_H

#include <engine/loader/scene.h>
#include <expected>
#include <memory>
#include <string>
#include <unordered_map>

#define ERROR_MESH_PATH "resources/error.obj"

struct aiNode;

namespace Engine::Manager {
    typedef std::shared_ptr<Loader::Scene> SharedScene;

    class SceneManager {
    private:
        std::unordered_map<std::string, SharedScene> scenes;
    public:
        SharedScene errorScene;

        SceneManager();

        std::expected<SharedScene, std::string> getScene(const std::string &scenePath);
        bool unloadScene(const std::string &scenePath);
        void clear();
    };

}

#endif
