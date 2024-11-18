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
    class SceneManager {
    private:
        std::unordered_map<std::string, Loader::Scene> scenes;
    public:
        Loader::Scene errorScene;

        SceneManager();

        std::expected<Loader::Scene, std::string> getScene(const std::string &scenePath);
        bool unloadScene(const std::string &scenePath);
        void clear();
    };

}

#endif
