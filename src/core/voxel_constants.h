/**************************************************************************/
/*  voxel_constants.h                                                     */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             KEEN VOXEL ENGINE                          */
/*                        https://keenvoxelengine.org                     */
/**************************************************************************/
/* Copyright (c) 2025-present Keen Voxel Engine                           */
/*                   All rights reserved.                                 */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifndef VOXEL_CONSTANTS_H
#define VOXEL_CONSTANTS_H

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

#endif // VOXEL_CONSTANTS_H