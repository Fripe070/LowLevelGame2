#ifndef TEXTURE_H
#define TEXTURE_H
#include <expected>
#include <string>

namespace Engine::Loader {
    /*!
     * Load a texture from a file.
     * @param filePath The path to the file.
     * @return The texture ID if successful, or an error message if not.
     * @attention If returned successfully, it is YOUR responsibility to free the memory allocated by opengl.
     */
    std::expected<unsigned int, std::string> loadTexture(const char* filePath);

    // TODO: Allow loading cubemap from equirectangular projection
    /*!
     * Loads a cubemap texture from a set of files.
     * @param filePath The path to the file. The different directions are inserted before the file extension with an underscore.
     * @return The texture ID if successful, or an error message if not.
     * @attention If returned successfully, it is YOUR responsibility to free the memory allocated by opengl.
     * @note The file name suffixes are: `_right`, `_left`, `_top`, `_bottom`, `_front`, `_back`.
     */
    std::expected<unsigned int, std::string> loadCubeMap(const std::string &filePath);
}

#endif //TEXTURE_H
