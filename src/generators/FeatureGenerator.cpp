/**************************************************************************/
/*  FeatureGenerator.cpp                                                  */
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

#include "FeatureGenerator.h"
#include "core/voxel.h"

// Godot includes
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace voxel_engine {

void FeatureGenerator::_bind_methods() {
	// Seed
	ClassDB::bind_method(D_METHOD("set_seed", "seed"), &FeatureGenerator::set_seed);
	ClassDB::bind_method(D_METHOD("get_seed"), &FeatureGenerator::get_seed);

	// Rule management
	ClassDB::bind_method(D_METHOD("add_rule", "feature_type", "spawn_probability", "allowed_biomes",
								 "min_height", "max_height", "voxel_type", "cluster_size", "min_spacing"),
			&FeatureGenerator::add_rule);
	ClassDB::bind_method(D_METHOD("remove_rule", "index"), &FeatureGenerator::remove_rule);
	ClassDB::bind_method(D_METHOD("clear_rules"), &FeatureGenerator::clear_rules);
	ClassDB::bind_method(D_METHOD("get_rule_count"), &FeatureGenerator::get_rule_count);
	ClassDB::bind_method(D_METHOD("get_rule", "index"), &FeatureGenerator::get_rule);

	// Preset rules
	ClassDB::bind_method(D_METHOD("add_tree_rule", "probability", "biomes", "min_height", "max_height"),
			&FeatureGenerator::add_tree_rule);
	ClassDB::bind_method(D_METHOD("add_ore_rule", "ore_voxel_type", "probability", "min_height", "max_height", "cluster_size"),
			&FeatureGenerator::add_ore_rule);
	ClassDB::bind_method(D_METHOD("add_rock_rule", "probability", "biomes", "min_height", "max_height"),
			&FeatureGenerator::add_rock_rule);
	ClassDB::bind_method(D_METHOD("add_foliage_rule", "foliage_voxel", "probability", "biomes"),
			&FeatureGenerator::add_foliage_rule);

	// Queries
	ClassDB::bind_method(D_METHOD("should_spawn_at", "x", "y", "z", "rule_index"),
			&FeatureGenerator::should_spawn_at);
	ClassDB::bind_method(D_METHOD("is_biome_allowed", "biome_index", "rule_index"),
			&FeatureGenerator::is_biome_allowed);

	// Feature generation
	ClassDB::bind_method(D_METHOD("get_features_in_region", "start_x", "start_y", "start_z",
								 "size_x", "size_y", "size_z", "surface_heights", "biome_indices"),
			&FeatureGenerator::get_features_in_region);
	ClassDB::bind_method(D_METHOD("get_feature_at", "x", "y", "z", "surface_height", "biome_index"),
			&FeatureGenerator::get_feature_at);
	ClassDB::bind_method(D_METHOD("generate_ore_cluster", "center_x", "center_y", "center_z", "cluster_size"),
			&FeatureGenerator::generate_ore_cluster);

	// Properties
	ADD_PROPERTY(PropertyInfo(Variant::INT, "seed"), "set_seed", "get_seed");

	// Bind FeatureType enum
	BIND_ENUM_CONSTANT(FEATURE_TREE);
	BIND_ENUM_CONSTANT(FEATURE_ORE_VEIN);
	BIND_ENUM_CONSTANT(FEATURE_ROCK);
	BIND_ENUM_CONSTANT(FEATURE_FOLIAGE);
	BIND_ENUM_CONSTANT(FEATURE_STRUCTURE);
}

FeatureGenerator::FeatureGenerator() : seed(DEFAULT_WORLD_SEED) {
}

FeatureGenerator::~FeatureGenerator() {
}

// Position-seeded hash function for deterministic generation
uint32_t FeatureGenerator::hash_position(int x, int y, int z, int seed) {
	// FNV-1a hash variant
	uint32_t hash = 2166136261u;
	hash ^= static_cast<uint32_t>(x);
	hash *= 16777619u;
	hash ^= static_cast<uint32_t>(y);
	hash *= 16777619u;
	hash ^= static_cast<uint32_t>(z);
	hash *= 16777619u;
	hash ^= static_cast<uint32_t>(seed);
	hash *= 16777619u;

	// Additional mixing
	hash ^= hash >> 16;
	hash *= 0x85ebca6bu;
	hash ^= hash >> 13;
	hash *= 0xc2b2ae35u;
	hash ^= hash >> 16;

	return hash;
}

float FeatureGenerator::hash_to_float(uint32_t hash) {
	// Convert hash to float in range [0, 1)
	return static_cast<float>(hash & 0x00FFFFFFu) / static_cast<float>(0x01000000u);
}

void FeatureGenerator::set_seed(int p_seed) {
	seed = p_seed;
}

int FeatureGenerator::get_seed() const {
	return seed;
}

void FeatureGenerator::add_rule(int feature_type, float spawn_probability,
		const TypedArray<int> &allowed_biomes,
		int min_height, int max_height,
		int voxel_type, int cluster_size, int min_spacing) {
	FeatureRule rule;
	rule.feature_type = static_cast<FeatureType>(feature_type);
	rule.spawn_probability = Math::clamp(spawn_probability, 0.0f, 1.0f);
	rule.allowed_biomes = allowed_biomes;
	rule.min_height = min_height;
	rule.max_height = max_height;
	rule.voxel_type = voxel_type;
	rule.cluster_size = Math::max(1, cluster_size);
	rule.min_spacing = Math::max(1, min_spacing);

	rules.push_back(rule);
}

void FeatureGenerator::remove_rule(int index) {
	if (index >= 0 && index < rules.size()) {
		rules.remove_at(index);
	}
}

void FeatureGenerator::clear_rules() {
	rules.clear();
}

int FeatureGenerator::get_rule_count() const {
	return rules.size();
}

Dictionary FeatureGenerator::get_rule(int index) const {
	Dictionary result;
	if (index >= 0 && index < rules.size()) {
		const FeatureRule &rule = rules[index];
		result["feature_type"] = static_cast<int>(rule.feature_type);
		result["spawn_probability"] = rule.spawn_probability;
		result["allowed_biomes"] = rule.allowed_biomes;
		result["min_height"] = rule.min_height;
		result["max_height"] = rule.max_height;
		result["voxel_type"] = rule.voxel_type;
		result["cluster_size"] = rule.cluster_size;
		result["min_spacing"] = rule.min_spacing;
	}
	return result;
}

void FeatureGenerator::add_tree_rule(float probability, const TypedArray<int> &biomes, int min_h, int max_h) {
	FeatureRule rule;
	rule.feature_type = FeatureType::TREE;
	rule.spawn_probability = probability;
	rule.allowed_biomes = biomes;
	rule.min_height = min_h;
	rule.max_height = max_h;
	rule.voxel_type = VoxelType::DIRT; // Placeholder - trees would need special handling
	rule.cluster_size = 1;
	rule.min_spacing = 4;

	rules.push_back(rule);
}

void FeatureGenerator::add_ore_rule(int ore_voxel_type, float probability, int min_h, int max_h, int cluster_size) {
	FeatureRule rule;
	rule.feature_type = FeatureType::ORE_VEIN;
	rule.spawn_probability = probability;
	// Ores spawn in all biomes by default (underground)
	rule.min_height = min_h;
	rule.max_height = max_h;
	rule.voxel_type = ore_voxel_type;
	rule.cluster_size = cluster_size;
	rule.min_spacing = 2;

	rules.push_back(rule);
}

void FeatureGenerator::add_rock_rule(float probability, const TypedArray<int> &biomes, int min_h, int max_h) {
	FeatureRule rule;
	rule.feature_type = FeatureType::ROCK;
	rule.spawn_probability = probability;
	rule.allowed_biomes = biomes;
	rule.min_height = min_h;
	rule.max_height = max_h;
	rule.voxel_type = VoxelType::STONE;
	rule.cluster_size = 3;
	rule.min_spacing = 8;

	rules.push_back(rule);
}

void FeatureGenerator::add_foliage_rule(int foliage_voxel, float probability, const TypedArray<int> &biomes) {
	FeatureRule rule;
	rule.feature_type = FeatureType::FOLIAGE;
	rule.spawn_probability = probability;
	rule.allowed_biomes = biomes;
	rule.min_height = 0;
	rule.max_height = 256;
	rule.voxel_type = foliage_voxel;
	rule.cluster_size = 1;
	rule.min_spacing = 1;

	rules.push_back(rule);
}

bool FeatureGenerator::should_spawn_at(int x, int y, int z, int rule_index) const {
	if (rule_index < 0 || rule_index >= rules.size()) {
		return false;
	}

	const FeatureRule &rule = rules[rule_index];

	// Check height bounds
	if (y < rule.min_height || y > rule.max_height) {
		return false;
	}

	// Use position hash for deterministic probability check
	uint32_t hash = hash_position(x, y, z, seed + rule_index);
	float probability = hash_to_float(hash);

	return probability < rule.spawn_probability;
}

bool FeatureGenerator::is_biome_allowed(int biome_index, int rule_index) const {
	if (rule_index < 0 || rule_index >= rules.size()) {
		return false;
	}

	const FeatureRule &rule = rules[rule_index];

	// If no biomes specified, allow all
	if (rule.allowed_biomes.size() == 0) {
		return true;
	}

	// Check if biome is in allowed list
	for (int i = 0; i < rule.allowed_biomes.size(); i++) {
		if (static_cast<int>(rule.allowed_biomes[i]) == biome_index) {
			return true;
		}
	}

	return false;
}

Array FeatureGenerator::get_features_in_region(int start_x, int start_y, int start_z,
		int size_x, int size_y, int size_z,
		const TypedArray<int> &surface_heights,
		const TypedArray<int> &biome_indices) const {
	Array features;

	// Iterate through all positions in the region
	for (int lx = 0; lx < size_x; lx++) {
		for (int lz = 0; lz < size_z; lz++) {
			int world_x = start_x + lx;
			int world_z = start_z + lz;

			// Get surface height and biome for this column
			int col_index = lx * size_z + lz;
			int surface_height = 0;
			int biome_index = 0;

			if (col_index < surface_heights.size()) {
				surface_height = surface_heights[col_index];
			}
			if (col_index < biome_indices.size()) {
				biome_index = biome_indices[col_index];
			}

			// Check each rule for surface features
			for (int rule_idx = 0; rule_idx < rules.size(); rule_idx++) {
				const FeatureRule &rule = rules[rule_idx];

				// Surface features spawn at surface height
				if (rule.feature_type == FeatureType::TREE ||
						rule.feature_type == FeatureType::ROCK ||
						rule.feature_type == FeatureType::FOLIAGE) {
					if (!is_biome_allowed(biome_index, rule_idx)) {
						continue;
					}

					if (should_spawn_at(world_x, surface_height, world_z, rule_idx)) {
						Dictionary feature;
						feature["position"] = Vector3i(world_x, surface_height + 1, world_z);
						feature["feature_type"] = static_cast<int>(rule.feature_type);
						feature["voxel_type"] = rule.voxel_type;
						feature["size"] = rule.cluster_size;
						features.push_back(feature);
					}
				}
			}

			// Check ore rules for underground features
			for (int ly = 0; ly < size_y; ly++) {
				int world_y = start_y + ly;

				// Skip if above or at surface (ores are underground)
				if (world_y >= surface_height) {
					continue;
				}

				for (int rule_idx = 0; rule_idx < rules.size(); rule_idx++) {
					const FeatureRule &rule = rules[rule_idx];

					if (rule.feature_type != FeatureType::ORE_VEIN) {
						continue;
					}

					if (should_spawn_at(world_x, world_y, world_z, rule_idx)) {
						Dictionary feature;
						feature["position"] = Vector3i(world_x, world_y, world_z);
						feature["feature_type"] = static_cast<int>(rule.feature_type);
						feature["voxel_type"] = rule.voxel_type;
						feature["size"] = rule.cluster_size;
						features.push_back(feature);
					}
				}
			}
		}
	}

	return features;
}

Dictionary FeatureGenerator::get_feature_at(int x, int y, int z, int surface_height, int biome_index) const {
	Dictionary result;

	for (int rule_idx = 0; rule_idx < rules.size(); rule_idx++) {
		const FeatureRule &rule = rules[rule_idx];

		// Check biome restriction
		if (!is_biome_allowed(biome_index, rule_idx)) {
			continue;
		}

		// Check height based on feature type
		bool height_valid = false;
		if (rule.feature_type == FeatureType::ORE_VEIN) {
			// Ores spawn underground
			height_valid = (y < surface_height && y >= rule.min_height && y <= rule.max_height);
		} else {
			// Surface features spawn at surface
			height_valid = (y == surface_height + 1 && surface_height >= rule.min_height && surface_height <= rule.max_height);
		}

		if (!height_valid) {
			continue;
		}

		if (should_spawn_at(x, y, z, rule_idx)) {
			result["position"] = Vector3i(x, y, z);
			result["feature_type"] = static_cast<int>(rule.feature_type);
			result["voxel_type"] = rule.voxel_type;
			result["size"] = rule.cluster_size;
			return result;
		}
	}

	return result; // Empty dictionary if no feature
}

Array FeatureGenerator::generate_ore_cluster(int center_x, int center_y, int center_z, int cluster_size) const {
	Array positions;

	// Generate a blob of ore around the center point
	for (int dx = -cluster_size; dx <= cluster_size; dx++) {
		for (int dy = -cluster_size; dy <= cluster_size; dy++) {
			for (int dz = -cluster_size; dz <= cluster_size; dz++) {
				// Calculate distance from center
				float dist = Math::sqrt(static_cast<float>(dx * dx + dy * dy + dz * dz));

				// Use hash to determine if this position is part of the cluster
				int px = center_x + dx;
				int py = center_y + dy;
				int pz = center_z + dz;

				uint32_t hash = hash_position(px, py, pz, seed + 12345);
				float threshold = hash_to_float(hash);

				// Higher probability closer to center
				float inclusion_prob = 1.0f - (dist / static_cast<float>(cluster_size + 1));

				if (threshold < inclusion_prob && dist <= cluster_size) {
					positions.push_back(Vector3i(px, py, pz));
				}
			}
		}
	}

	return positions;
}

} // namespace voxel_engine
