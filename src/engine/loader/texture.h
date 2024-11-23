#ifndef TEXTURE_H
#define TEXTURE_H
#include <expected>
#include <string>

namespace Engine::Loader {
    /*!
     * Load a texture from a file.
     * @param filePath The path to the file.
     * @return The texture ID if successful, or an error message if not.
     * @note If returned successfully, it is YOUR responsibility to free the allocated memory.
     */
    std::expected<unsigned int, std::string> loadTexture(const char* filePath);
}

#endif //TEXTURE_H
