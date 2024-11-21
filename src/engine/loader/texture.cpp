#define STB_IMAGE_IMPLEMENTATION

#include "texture.h"
#include <engine/logging.h>
#include <stb_image.h>
#include <gl/glew.h>
#ifndef NDEBUG
#include <chrono>
#endif

namespace Engine::Loader {
    std::expected<unsigned int, std::string> loadTexture(const char *filePath) {
#ifndef NDEBUG
        const auto startTimer = std::chrono::high_resolution_clock::now();
#endif
        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, channelCount;
        stbi_uc *imgData = stbi_load(filePath, &width, &height, &channelCount, 0);
        if (!imgData) {
            logError("Failed to load texture \"%s\": %s", filePath, stbi_failure_reason());
            return std::unexpected(FILE_REF "Failed to load texture: " + std::string(stbi_failure_reason()));
        }
#ifndef NDEBUG
        logDebug("Loaded texture \"%s\" with dimensions %dx%d and %d channels in %dms",
            filePath, width, height, channelCount,
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTimer).count());
#endif

        GLint format;
        switch (channelCount) {
            case 1: format = GL_RED; break;
            case 3: format = GL_RGB; break;
            case 4: format = GL_RGBA; break;
            default: {
                logWarn("Unknown channel count %d, defaulting to RGB", channelCount);
                format = GL_RGB;
            }
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, imgData);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(imgData);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        return textureID;
    }

    std::expected<unsigned int, std::string> loadTexture(const std::string &filePath) {
        return loadTexture(filePath.c_str());
    }
}