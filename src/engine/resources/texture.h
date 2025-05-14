#pragma once
#include <expected>
#include <string>

#include "engine/util/error.h"

namespace Resource
{
    /*!
     * Class that stores an OpenGL texture ID and deletes it when it goes out of scope
     */
    class ManagedTexture {
    public:
        unsigned int textureID;
        explicit ManagedTexture(unsigned int textureID);
        ~ManagedTexture();
    };
}

namespace Resource::Loading
{
    /*!
     * Load a texture from a file.
     * @param filePath The path to the file.
     * @return The texture ID if successful, or an error if not.
     * @attention If returned successfully, it is YOUR responsibility to free the memory allocated by opengl.
     */
    std::expected<unsigned int, Error> loadTexture(const char* filePath);
    /*!
     * Load a texture from memory.
     * @param data Byte array of the image data.
     * @param size The size of the byte array.
     * @return The texture ID if successful, or an error if not.
     * @attention If returned successfully, it is YOUR responsibility to free the memory allocated by opengl.
     */
    std::expected<unsigned int, Error> loadTexture(const unsigned char* data, int size);

    // TODO: Allow loading cubemap from equirectangular projection
    /*!
     * Loads a cubemap texture from a set of files.
     * @param filePath The path to the file. The different directions are inserted before the file extension with an underscore.
     * @return The texture ID if successful, or an error if not.
     * @attention If returned successfully, it is YOUR responsibility to free the memory allocated by opengl.
     * @note The file name suffixes are: `_right`, `_left`, `_top`, `_bottom`, `_front`, `_back`.
     */
    std::expected<unsigned int, Error> loadCubemap(const std::string& filePath);
    /*!
     * Loads a cubemap texture from a set of data.
     * @param data The data for each side of the cubemap.
     * @param sizes The sizes of each side of the cubemap.
     * @return The texture ID if successful, or an error if not.
     * @attention If returned successfully, it is YOUR responsibility to free the memory allocated by opengl.
     */
    std::expected<unsigned int, Error> loadCubemap(
        const std::array<const unsigned char*, 6>& data,
        const std::array<int, 6>& sizes);

    /*!
     * Loads a cubemap texture from a single file to be used for all sides.
     * @param filePath The path to the file.
     * @return The texture ID if successful, or an error if not.
     * @attention If returned successfully, it is YOUR responsibility to free the memory allocated by opengl.
     */
    std::expected<unsigned int, Error> loadCubemapSingle(const std::string& filePath);
    /*!
     * Loads a cubemap texture from a single byte array to be used for all sides.
     * @param data Byte array of the image data.
     * @param size The size of the byte array.
     * @return The texture ID if successful, or an error if not.
     * @attention If returned successfully, it is YOUR responsibility to free the memory allocated by opengl.
     */
    std::expected<unsigned int, Error> loadCubemapSingle(const unsigned char* data, int size);
}
