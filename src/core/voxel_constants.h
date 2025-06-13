#pragma once

namespace voxel_engine
{

    // Constants for voxel/chunk configuration
    constexpr int DEFAULT_CHUNK_SIZE = 8;
    constexpr int MAX_CHUNK_SIZE = 64;
    constexpr int MIN_CHUNK_SIZE = 8;

    // Material categories for voxel types
    enum class MaterialCategory
    {
        AIR,
        GAS,
        LIQUID,
        SOLID
    };

    // Voxel state constants
    constexpr int VOXEL_STATE_AIR = 0;
    constexpr int VOXEL_STATE_SOLID = 1;
    constexpr int VOXEL_STATE_WATER = 2;
    constexpr int VOXEL_STATE_CUSTOM_START = 8;

    // Voxel property bit flags
    constexpr unsigned int VOXEL_PROPERTY_TRANSPARENT = 1 << 0;
    constexpr unsigned int VOXEL_PROPERTY_LIQUID = 1 << 1;
    constexpr unsigned int VOXEL_PROPERTY_FOLIAGE = 1 << 2;
    constexpr unsigned int VOXEL_PROPERTY_EMISSIVE = 1 << 3;
    constexpr unsigned int VOXEL_PROPERTY_COLLIDABLE = 1 << 4;
    constexpr unsigned int VOXEL_PROPERTY_OPAQUE = 1 << 5;

    // LOD constants
    constexpr int MAX_LOD_LEVELS = 8;
    constexpr float DEFAULT_LOD_DISTANCE_MULTIPLIER = 2.0f;

    // Meshing constants
    constexpr float MESHING_ISOLEVEL = 0.5f; // For Marching Cubes algorithm

    // Generation constants
    constexpr int DEFAULT_WORLD_SEED = 1234;
    constexpr int DEFAULT_WORLD_HEIGHT = 256;
    constexpr float TERRAIN_SCALE = 0.01f;

} // namespace voxel_engine

// VOXEL_CONSTANTS_H