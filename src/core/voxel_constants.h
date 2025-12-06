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

#include <godot_cpp/variant/string.hpp>

namespace voxel_engine {

// Constants for voxel/chunk configuration
constexpr int DEFAULT_CHUNK_SIZE = 8;
constexpr int MAX_CHUNK_SIZE = 64;
constexpr int MIN_CHUNK_SIZE = 8;

// Biome blending constants
constexpr int MIN_BLEND_DISTANCE = 8;
constexpr int MAX_BLEND_DISTANCE = 32;
constexpr int DEFAULT_BLEND_DISTANCE = 8;

// Material categories for voxel types
enum class MaterialCategory {
	AIR,
	GAS,
	LIQUID,
	SOLID
};

// Integer constants for GDScript binding
constexpr int MATERIAL_CATEGORY_AIR = static_cast<int>(MaterialCategory::AIR);
constexpr int MATERIAL_CATEGORY_GAS = static_cast<int>(MaterialCategory::GAS);
constexpr int MATERIAL_CATEGORY_LIQUID = static_cast<int>(MaterialCategory::LIQUID);
constexpr int MATERIAL_CATEGORY_SOLID = static_cast<int>(MaterialCategory::SOLID);

// Voxel state constants
constexpr int VOXEL_STATE_AIR = 0;
constexpr int VOXEL_STATE_SOLID = 1;
constexpr int VOXEL_STATE_WATER = 2;
constexpr int VOXEL_STATE_CUSTOM_START = 8;

// Voxel property bit flags
constexpr unsigned int VOXEL_PROPERTY_NONE = 0;
constexpr unsigned int VOXEL_PROPERTY_TRANSPARENT = 1 << 0;
constexpr unsigned int VOXEL_PROPERTY_LIQUID = 1 << 1;
constexpr unsigned int VOXEL_PROPERTY_FOLIAGE = 1 << 2;
constexpr unsigned int VOXEL_PROPERTY_EMISSIVE = 1 << 3;
constexpr unsigned int VOXEL_PROPERTY_COLLIDABLE = 1 << 4;
constexpr unsigned int VOXEL_PROPERTY_OPAQUE = 1 << 5;
constexpr unsigned int VOXEL_PROPERTY_SOLID = 1 << 6;

// LOD constants
constexpr int MAX_LOD_LEVELS = 8;
constexpr float DEFAULT_LOD_DISTANCE_MULTIPLIER = 2.0f;

// Meshing constants
constexpr float MESHING_ISOLEVEL = 0.5f; // For Marching Cubes algorithm
constexpr int MAX_VERTICES = 16777216; // 16M vertices for VOXELS_FIRST mode
constexpr int MAX_VERTICES_HEIGHTMAP = 134217728; // 128M vertices for HEIGHTMAP_FIRST mode (8x)

// Generation constants
constexpr int DEFAULT_WORLD_SEED = 1234;
constexpr int DEFAULT_WORLD_HEIGHT = 256;
constexpr float TERRAIN_SCALE = 0.01f;

// VoxelDefinition - Data structure for voxel type properties
struct VoxelDefinition {
	int type_id; // Unique identifier matching VoxelType enum
	godot::String name; // Human-readable name
	MaterialCategory category; // Material category (AIR, GAS, LIQUID, SOLID)
	unsigned int property_flags; // Bitfield of VOXEL_PROPERTY_* flags
	float hardness; // Mining/breaking difficulty (0.0 = instant, higher = harder)
	float light_emission; // Light level emitted (0.0 = none, 1.0 = max)
	int material_id; // Placeholder ID for future material system

	VoxelDefinition() :
			type_id(0),
			name("Unknown"),
			category(MaterialCategory::AIR),
			property_flags(VOXEL_PROPERTY_NONE),
			hardness(0.0f),
			light_emission(0.0f),
			material_id(0) {}

	VoxelDefinition(int p_type_id, const godot::String &p_name, MaterialCategory p_category,
			unsigned int p_flags, float p_hardness, float p_emission, int p_material_id) :
			type_id(p_type_id),
			name(p_name),
			category(p_category),
			property_flags(p_flags),
			hardness(p_hardness),
			light_emission(p_emission),
			material_id(p_material_id) {}

	// Convenience methods
	bool is_solid() const { return (property_flags & VOXEL_PROPERTY_SOLID) != 0; }
	bool is_transparent() const { return (property_flags & VOXEL_PROPERTY_TRANSPARENT) != 0; }
	bool is_liquid() const { return (property_flags & VOXEL_PROPERTY_LIQUID) != 0; }
	bool is_collidable() const { return (property_flags & VOXEL_PROPERTY_COLLIDABLE) != 0; }
	bool is_emissive() const { return (property_flags & VOXEL_PROPERTY_EMISSIVE) != 0; }
	bool has_property(unsigned int flag) const { return (property_flags & flag) != 0; }
};

// Feature types for FeatureGenerator
enum class FeatureType {
	TREE,
	ORE_VEIN,
	ROCK,
	FOLIAGE,
	STRUCTURE
};

} // namespace voxel_engine

#endif // VOXEL_CONSTANTS_H