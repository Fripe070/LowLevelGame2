#pragma once
#include <array>

#define BUFFERS_MV_FROM_TO(from, to) \
    to->VAO = from.VAO; \
    to->VBO = from.VBO; \
    to->EBO = from.EBO; \
    from.VAO = 0; \
    from.VBO = 0; \
    from.EBO = 0;


enum CubeVertIndex {
    BOTTOM_LEFT_FRONT = 0,
    BOTTOM_RIGHT_FRONT = 1,
    TOP_LEFT_FRONT = 2,
    TOP_RIGHT_FRONT = 3,
    BOTTOM_LEFT_BACK = 4,
    BOTTOM_RIGHT_BACK = 5,
    TOP_LEFT_BACK = 6,
    TOP_RIGHT_BACK = 7,
};
/*!
 * @returns The vertices and indices for a cube with the given dimensions.
 */
constexpr std::array<float, 8*3>
getCubeVertices(const float x1, const float y1, const float z1, const float x2, const float y2, const float z2) {
    return {
        x1, y1, z1, // BOT LEF FRO
        x2, y1, z1, // BOT RIG FRO
        x1, y2, z1, // TOP LEF FRO
        x2, y2, z1, // TOP RIG FRO
        x1, y1, z2, // BOT LEF BAC
        x2, y1, z2, // BOT RIG BAC
        x1, y2, z2, // TOP LEF BAC
        x2, y2, z2, // TOP RIG BAC
    };
}

// 6 faces, 2 triangles per face, 3 vertices per triangle
constexpr std::array<unsigned int, 6 * 2 * 3> CubeIndicesInside = {
    // Front face
    BOTTOM_LEFT_FRONT,  BOTTOM_RIGHT_FRONT, TOP_LEFT_FRONT,
    BOTTOM_RIGHT_FRONT, TOP_RIGHT_FRONT,    TOP_LEFT_FRONT,
    // Back face
    BOTTOM_RIGHT_BACK,  BOTTOM_LEFT_BACK,   TOP_RIGHT_BACK,
    BOTTOM_LEFT_BACK,   TOP_LEFT_BACK,      TOP_RIGHT_BACK,
    // Left face
    BOTTOM_LEFT_BACK,   BOTTOM_LEFT_FRONT,  TOP_LEFT_BACK,
    BOTTOM_LEFT_FRONT,  TOP_LEFT_FRONT,     TOP_LEFT_BACK,
    // Right face
    BOTTOM_RIGHT_FRONT, BOTTOM_RIGHT_BACK,  TOP_RIGHT_FRONT,
    BOTTOM_RIGHT_BACK,  TOP_RIGHT_BACK,     TOP_RIGHT_FRONT,
    // Top face
    TOP_LEFT_FRONT,     TOP_RIGHT_FRONT,    TOP_LEFT_BACK,
    TOP_RIGHT_FRONT,    TOP_RIGHT_BACK,     TOP_LEFT_BACK,
    // Bottom face
    BOTTOM_RIGHT_FRONT, BOTTOM_LEFT_FRONT,  BOTTOM_RIGHT_BACK,
    BOTTOM_LEFT_FRONT,  BOTTOM_LEFT_BACK,   BOTTOM_RIGHT_BACK
};

constexpr std::array<float, 4*3> ScreenSpaceQuadVertices = {
    -1.0f,  1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f,
     1.0f,  1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,
};
constexpr std::array<unsigned int, 2*3> ScreenSpaceQuadIndices = {
    0, 1, 2,
    1, 3, 2,
};

