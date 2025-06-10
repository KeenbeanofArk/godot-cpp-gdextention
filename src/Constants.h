// Constants.h
#pragma once

#include <array>
#include <vector>

namespace Constants {

    // Edge-to-corner mapping (12 edges per cube)
    static constexpr std::array<int, 12> cornerIndexAFromEdge = {
        0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3
    };

    static constexpr std::array<int, 12> cornerIndexBFromEdge = {
        1, 2, 3, 0, 5, 6, 7, 4, 4, 5, 6, 7
    };

    // Marching Cubes triangle lookup table (partial entries shown)
    static const std::vector<std::array<int, 16>> marching_triangles = {
        std::array<int,16>{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        std::array<int,16>{  0,  8,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        std::array<int,16>{  0,  1,  9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        std::array<int,16>{  1,  8,  3,  9,  8,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        std::array<int,16>{  1,  2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        std::array<int,16>{  0,  8,  3,  1,  2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        std::array<int,16>{  9,  2, 10,  0,  2,  9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        std::array<int,16>{  2,  8,  3, 10,  8,  2, 10,  9,  8, -1, -1, -1, -1, -1, -1, -1 },
        std::array<int,16>{  3, 11,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        std::array<int,16>{  0, 11,  2,  8, 11,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        std::array<int,16>{  1,  9,  0,  2,  3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        std::array<int,16>{  1, 11,  2,  1,  9, 11,  9,  8, 11, -1, -1, -1, -1, -1, -1, -1 },
        std::array<int,16>{ 10,  3, 11,  1,  3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        std::array<int,16>{  0, 10,  1,  0,  8, 10,  8, 11, 10, -1, -1, -1, -1, -1, -1, -1 },
        std::array<int,16>{  3,  9,  0,  3, 11,  9, 11, 10,  9, -1, -1, -1, -1, -1, -1, -1 },
        std::array<int,16>{  9,  8, 10, 10,  8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }
        // Add remaining 240 entries here...
    };

} // namespace Constants