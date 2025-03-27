#pragma once
#include <expected>
#include <string>
#include <engine/util/error.h>

namespace Engine {
    enum TextureType {
        TEXTURE_2D,
        CUBEMAP
    };

    /*!
     * Class that stores an OpenGL texture ID and deletes it when it goes out of scope
     */
    class ManagedTexture {
    public:
        unsigned int textureID;
        explicit ManagedTexture(unsigned int textureID);
        ~ManagedTexture();
    };

    namespace Loader {
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
        std::expected<unsigned int, Error> loadCubeMap(const std::string &filePath);
    }
}
