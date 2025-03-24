#pragma once
#include <expected>
#include <memory>
#include <string>
#include <unordered_map>

#include "engine/loader/scene.h"

#define ERROR_MESH_PATH "resources/assets/models/error.obj"

struct aiNode;

namespace Engine::Manager {
    // TODO: Document like I have in texture manager
    // TODO: Hot reloading
    // TODO: Maybe a tad *too* ready to unload unused assets?
    //  We might need to implement our own reference counting system
    class SceneManager {
    private:
        std::unordered_map<std::string, std::weak_ptr<Loader::Scene>> scenes;
    public:
        std::shared_ptr<Loader::Scene> errorScene;

        SceneManager();

        std::expected<std::shared_ptr<Loader::Scene>, std::string> getScene(const std::string &scenePath);
    };

}

