#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "engine/util/error.h"
#include "engine/util/logging.h"

namespace Resource {
    ManagedTexture::ManagedTexture(const unsigned int textureID): textureID(textureID) {}
    ManagedTexture::~ManagedTexture() {
        glDeleteTextures(1, &textureID);
    }
}

namespace Resource::Loading {
    struct ImageData {
        int width, height, channelCount;
        stbi_uc *imgData;
    };
    /*!
     * Internal helper function to load an image from a file.
     * @attention You as the caller are responsible for freeing the allocated memory using `stbi_image_free`.
    */
    Expected<ImageData> loadImage(const char *filePath) {
        int width, height, channelCount;
        stbi_uc *imgData = stbi_load(filePath, &width, &height, &channelCount, 0);
        if (!imgData) {
            stbi_image_free(imgData);
            return std::unexpected(ERROR(
                std::string("Failed to load texture: \"") + filePath + "\": " + stbi_failure_reason()));
        }
        return ImageData{width, height, channelCount, imgData};
    }
    Expected<ImageData> loadImageMemory(const unsigned char *data, const int size) {
        int width, height, channelCount;
        stbi_uc *imgData = stbi_load_from_memory(data, size, &width, &height, &channelCount, 0);
        if (!imgData) {
            stbi_image_free(imgData);
            return std::unexpected(ERROR(
                std::string("Failed to load texture from memory: ") + stbi_failure_reason()));
        }
        return ImageData{width, height, channelCount, imgData};
    }

    GLint getGLChannels(const int channelCount) {
        switch (channelCount) {
            case 1: return GL_RED;
            case 3: return GL_RGB;
            case 4: return GL_RGBA;
            default: {
                SPDLOG_WARN("Unknown channel count %d, defaulting to single channel", channelCount);
                return GL_RED;  // Least likely to segfault :+1:
            }
        }
    }

    std::expected<unsigned int, Error> loadTexture(const ImageData& imgData) {
        const GLint format = getGLChannels(imgData.channelCount);

        unsigned int textureID;
        glGenTextures(1, &textureID);

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, imgData.width, imgData.height, 0, format, GL_UNSIGNED_BYTE, imgData.imgData);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);  // TODO: GL_CLAMP_TO_EDGE to better support alpha textures?
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        return textureID;
    }

    std::expected<unsigned int, Error> loadTexture(const char* filePath)
    {
        Expected<ImageData> imgData = loadImage(filePath);
        if (!imgData)
            return std::unexpected(FW_ERROR(imgData.error(), "Failed to load texture"));
        const std::expected<unsigned int, Error> texture = loadTexture(imgData.value());
        SPDLOG_TRACE("Loaded texture \"{}\" with dimensions {}x{}", filePath, imgData->width, imgData->height);
        stbi_image_free(imgData->imgData);
        return texture;
    }

    std::expected<unsigned int, Error> loadTexture(const unsigned char* data, const int size)
    {
        Expected<ImageData> imgData = loadImageMemory(data, size);
        if (!imgData)
            return std::unexpected(FW_ERROR(imgData.error(), "Failed to load texture from memory"));
        const std::expected<unsigned int, Error> texture = loadTexture(imgData.value());
        SPDLOG_TRACE("Loaded texture from memory with dimensions {}x{}", imgData->width, imgData->height);
        stbi_image_free(imgData->imgData);
        return texture;
    }

    constexpr std::array<std::string, 6> cubemapFaces = {"right", "left", "top", "bottom", "front", "back"};

    std::expected<unsigned int, Error> loadCubemap(const std::string& filePath) {
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

        const auto extension_index = filePath.find_last_of('.');
        const auto pathPrefix = filePath.substr(0, extension_index) + "_";

        ImageData firstData = {};
        for (int i = 0; i < 6; i++) {
            const std::string path = pathPrefix + cubemapFaces[i] + filePath.substr(extension_index);

            Expected<ImageData> imgData = loadImage(path.c_str());
            if (!imgData) {
                glDeleteTextures(1, &textureID);
                return std::unexpected(FW_ERROR(imgData.error(), "Failed to load cubemap texture"));
            }
            if (imgData->width != imgData->height) {
                glDeleteTextures(1, &textureID);
                stbi_image_free(imgData->imgData);
                return std::unexpected(ERROR("Cubemap texture must be square"));
            }
            if (i == 0)
                firstData = imgData.value();
            else if (imgData->width != firstData.width || imgData->channelCount != firstData.channelCount) {
                glDeleteTextures(1, &textureID);
                stbi_image_free(imgData->imgData);
                return std::unexpected(ERROR("Cubemap texture faces must have the same dimensions and channel counts"));
            }
            SPDLOG_DEBUG("Loaded cubemap %s texture \"%s\" with dimensions %dx%d",
                cubemapFaces[i].c_str(), path.c_str(), imgData->width, imgData->height);

            const GLint faceDir = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
            assert(faceDir >= GL_TEXTURE_CUBE_MAP_POSITIVE_X && faceDir <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);

            const GLint format = getGLChannels(imgData->channelCount);
            glTexImage2D(faceDir,
                0, format, imgData->width, imgData->height, 0, format, GL_UNSIGNED_BYTE, imgData->imgData);
            stbi_image_free(imgData->imgData);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        return textureID;
    }
    std::expected<unsigned int, Error> loadCubemap(const std::array<const unsigned char*, 6>& data, const std::array<int, 6>& sizes)
    {
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

        ImageData firstData = {};
        for (int i = 0; i < 6; i++) {
            Expected<ImageData> imgData = loadImageMemory(data[i], sizes[i]);
            if (!imgData) {
                glDeleteTextures(1, &textureID);
                return std::unexpected(FW_ERROR(imgData.error(), "Failed to load cubemap texture"));
            }
            if (imgData->width != imgData->height) {
                glDeleteTextures(1, &textureID);
                stbi_image_free(imgData->imgData);
                return std::unexpected(ERROR("Cubemap texture must be square"));
            }
            if (i == 0)
                firstData = imgData.value();
            else if (imgData->width != firstData.width || imgData->channelCount != firstData.channelCount) {
                glDeleteTextures(1, &textureID);
                stbi_image_free(imgData->imgData);
                return std::unexpected(ERROR("Cubemap texture faces must have the same dimensions and channel counts"));
            }
            SPDLOG_DEBUG("Loaded cubemap %d texture with dimensions %dx%d",
                cubemapFaces[i].c_str(), imgData->width, imgData->height);

            const GLint faceDir = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
            assert(faceDir >= GL_TEXTURE_CUBE_MAP_POSITIVE_X && faceDir <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);

            const GLint format = getGLChannels(imgData->channelCount);
            glTexImage2D(faceDir,
                0, format, imgData->width, imgData->height, 0, format, GL_UNSIGNED_BYTE, imgData->imgData);
            stbi_image_free(imgData->imgData);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        return textureID;
    }

    std::expected<unsigned int, Error> loadCubemapSingle(const std::string& filePath)
    {
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

        Expected<ImageData> imgData = loadImage(filePath.c_str());
        if (!imgData) {
            glDeleteTextures(1, &textureID);
            return std::unexpected(FW_ERROR(imgData.error(), "Failed to load cubemap texture"));
        }
        if (imgData->width != imgData->height) {
            glDeleteTextures(1, &textureID);
            stbi_image_free(imgData->imgData);
            return std::unexpected(ERROR("Cubemap texture must be square"));
        }

        const GLint format = getGLChannels(imgData->channelCount);
        for (int i = 0; i < 6; i++) {
            const GLint faceDir = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
            assert(faceDir >= GL_TEXTURE_CUBE_MAP_POSITIVE_X && faceDir <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
            glTexImage2D(faceDir,
                0, format, imgData->width, imgData->height, 0, format, GL_UNSIGNED_BYTE, imgData->imgData);
        }
        stbi_image_free(imgData->imgData);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        return textureID;
    }

    std::expected<unsigned int, Error> loadCubemapSingle(const unsigned char* data, int size)
    {
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

        Expected<ImageData> imgData = loadImageMemory(data, size);
        if (!imgData) {
            glDeleteTextures(1, &textureID);
            return std::unexpected(FW_ERROR(imgData.error(), "Failed to load cubemap texture"));
        }
        if (imgData->width != imgData->height) {
            glDeleteTextures(1, &textureID);
            stbi_image_free(imgData->imgData);
            return std::unexpected(ERROR("Cubemap texture must be square"));
        }

        const GLint format = getGLChannels(imgData->channelCount);
        for (int i = 0; i < 6; i++) {
            const GLint faceDir = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
            assert(faceDir >= GL_TEXTURE_CUBE_MAP_POSITIVE_X && faceDir <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
            glTexImage2D(faceDir,
                0, format, imgData->width, imgData->height, 0, format, GL_UNSIGNED_BYTE, imgData->imgData);
        }
        stbi_image_free(imgData->imgData);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        return textureID;
    }
}