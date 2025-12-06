/**************************************************************************/
/*  FeatureGenerator.h                                                    */
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

#ifndef FEATURE_GENERATOR_H
#define FEATURE_GENERATOR_H

#include "core/voxel_constants.h"

// Godot includes
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/vector3i.hpp>

using namespace godot;

namespace voxel_engine {

/// @brief FeatureRule - Defines a decoration/feature placement rule.
struct FeatureRule {
	FeatureType feature_type; // Type of feature (TREE, ORE_VEIN, etc.)
	float spawn_probability; // Probability of spawning (0.0 - 1.0)
	TypedArray<int> allowed_biomes; // Biome indices where this feature can spawn
	int min_height; // Minimum Y coordinate for spawning
	int max_height; // Maximum Y coordinate for spawning
	int voxel_type; // Voxel type to place (for simple features like ores)
	int cluster_size; // Size of clusters (for ore veins, rocks)
	int min_spacing; // Minimum distance between features of same type

	FeatureRule() :
			feature_type(FeatureType::ROCK),
			spawn_probability(0.1f),
			min_height(0),
			max_height(256),
			voxel_type(0),
			cluster_size(1),
			min_spacing(4) {}

	FeatureRule(FeatureType p_type, float p_prob, int p_min_h, int p_max_h,
			int p_voxel, int p_cluster, int p_spacing) :
			feature_type(p_type),
			spawn_probability(p_prob),
			min_height(p_min_h),
			max_height(p_max_h),
			voxel_type(p_voxel),
			cluster_size(p_cluster),
			min_spacing(p_spacing) {}
};

/// @brief FeaturePlacement - Represents a placed feature instance.
struct FeaturePlacement {
	Vector3i position; // World position of the feature
	FeatureType feature_type; // Type of feature
	int voxel_type; // Voxel type to place
	int size; // Size/radius of the feature

	FeaturePlacement() :
			position(Vector3i(0, 0, 0)),
			feature_type(FeatureType::ROCK),
			voxel_type(0),
			size(1) {}

	FeaturePlacement(const Vector3i &p_pos, FeatureType p_type, int p_voxel, int p_size) :
			position(p_pos),
			feature_type(p_type),
			voxel_type(p_voxel),
			size(p_size) {}
};

/// @brief FeatureGenerator - Generates decorations and features.
/// Operates as a sibling to BiomeGenerator, coordinated by VoxelGenerator.
/// Features are placed AFTER terrain generation.
class FeatureGenerator : public Resource {
	GDCLASS(FeatureGenerator, Resource);

private:
	Vector<FeatureRule> rules;
	int seed;

	// Position-seeded deterministic hash function
	static uint32_t hash_position(int x, int y, int z, int seed);
	static float hash_to_float(uint32_t hash);

protected:
	static void _bind_methods();

public:
	FeatureGenerator();
	~FeatureGenerator();

	// Seed management
	void set_seed(int p_seed);
	int get_seed() const;

	// Rule management
	void add_rule(int feature_type, float spawn_probability,
			const TypedArray<int> &allowed_biomes,
			int min_height, int max_height,
			int voxel_type, int cluster_size, int min_spacing);
	void remove_rule(int index);
	void clear_rules();
	int get_rule_count() const;
	Dictionary get_rule(int index) const;

	// Add preset rules for common features
	void add_tree_rule(float probability, const TypedArray<int> &biomes, int min_h, int max_h);
	void add_ore_rule(int ore_voxel_type, float probability, int min_h, int max_h, int cluster_size);
	void add_rock_rule(float probability, const TypedArray<int> &biomes, int min_h, int max_h);
	void add_foliage_rule(int foliage_voxel, float probability, const TypedArray<int> &biomes);

	// Feature placement queries
	bool should_spawn_at(int x, int y, int z, int rule_index) const;
	bool is_biome_allowed(int biome_index, int rule_index) const;

	// Get features for a region (chunk)
	Array get_features_in_region(int start_x, int start_y, int start_z,
			int size_x, int size_y, int size_z,
			const TypedArray<int> &surface_heights,
			const TypedArray<int> &biome_indices) const;

	// Get a single feature placement at a position (returns Dictionary or empty)
	Dictionary get_feature_at(int x, int y, int z, int surface_height, int biome_index) const;

	// Utility: Generate ore cluster positions around a center point
	Array generate_ore_cluster(int center_x, int center_y, int center_z, int cluster_size) const;

	// Bind FeatureType enum constants
	enum FeatureTypeEnum {
		FEATURE_TREE = static_cast<int>(FeatureType::TREE),
		FEATURE_ORE_VEIN = static_cast<int>(FeatureType::ORE_VEIN),
		FEATURE_ROCK = static_cast<int>(FeatureType::ROCK),
		FEATURE_FOLIAGE = static_cast<int>(FeatureType::FOLIAGE),
		FEATURE_STRUCTURE = static_cast<int>(FeatureType::STRUCTURE)
	};
};

} // namespace voxel_engine

VARIANT_ENUM_CAST(voxel_engine::FeatureGenerator::FeatureTypeEnum);

#endif // FEATURE_GENERATOR_H
