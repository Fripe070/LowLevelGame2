#pragma once

#include <memory>
#include "shader.h"
#include "texture.h"

namespace Resource
{
    /*! A fairly light wrapper around a shader containing PBR material data to pass to it. */
    struct PBRMaterial {
        std::shared_ptr<Shader> shader;

        std::shared_ptr<ManagedTexture> albedo{};
        std::shared_ptr<ManagedTexture> normal{};
        std::shared_ptr<ManagedTexture> roughness{};
        std::shared_ptr<ManagedTexture> metallic{};
        std::shared_ptr<ManagedTexture> ambientOcclusion{};
    };
}
