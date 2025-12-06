/**************************************************************************/
/*  BiomeGenerator.h                                                      */
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

#ifndef BIOME_GENERATOR_H
#define BIOME_GENERATOR_H

#include "NoiseGenerator.h"
#include "core/voxel.h"
#include "core/voxel_constants.h"

// Godot includes
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/typed_array.hpp>

using namespace godot;

namespace voxel_engine {

struct BiomeData {
	String name;
	float min_height;
	float max_height;
	float min_temperature;
	float max_temperature;
	float min_humidity;
	float max_humidity;
	TypedArray<int32_t> surface_blocks;
	TypedArray<int32_t> subsurface_blocks;
	int depth;

	// New layer fields
	int bedrock_block; // Block type at bottom of world
	int filler_block; // Block type between subsurface and bedrock

	BiomeData() : min_height(-1.0f),
				  max_height(1.0f),
				  min_temperature(-1.0f),
				  max_temperature(1.0f),
				  min_humidity(-1.0f),
				  max_humidity(1.0f),
				  depth(4),
				  bedrock_block(VoxelType::STONE),
				  filler_block(VoxelType::STONE) {
	}
};

/// @brief BiomeWeight - Represents a biome's contribution during blending.
struct BiomeWeight {
	int biome_index;
	float weight;

	BiomeWeight() : biome_index(-1), weight(0.0f) {}
	BiomeWeight(int idx, float w) : biome_index(idx), weight(w) {}
};

class BiomeGenerator : public Resource {
	GDCLASS(BiomeGenerator, Resource);

private:
	Ref<NoiseGenerator> height_noise;
	Ref<NoiseGenerator> temperature_noise;
	Ref<NoiseGenerator> humidity_noise;

	Vector<BiomeData> biomes;
	float sea_level;
	int default_voxel;
	int seed;

	// Biome blending settings
	int blend_distance; // Distance in blocks for biome transitions (8-32)

	// Position-seeded hash for deterministic selection
	static uint32_t hash_position(int x, int y, int z, int seed);
	static float hash_to_float(uint32_t hash);
	static int hash_to_index(uint32_t hash, int count);

protected:
	static void _bind_methods();

public:
	BiomeGenerator();
	~BiomeGenerator();

	// Noise setters and getters
	void set_height_noise(const Ref<NoiseGenerator> &p_noise);
	Ref<NoiseGenerator> get_height_noise() const;

	void set_temperature_noise(const Ref<NoiseGenerator> &p_noise);
	Ref<NoiseGenerator> get_temperature_noise() const;

	void set_humidity_noise(const Ref<NoiseGenerator> &p_noise);
	Ref<NoiseGenerator> get_humidity_noise() const;

	// Sea level control
	void set_sea_level(float p_sea_level);
	float get_sea_level() const;

	// Seed control
	void set_seed(int p_seed);
	int get_seed() const;

	// Default voxel for empty spaces
	void set_default_voxel(int p_voxel_id);
	int get_default_voxel() const;

	// Blend distance (8-32 blocks)
	void set_blend_distance(int p_distance);
	int get_blend_distance() const;

	// Biome management
	void add_biome(const String &name,
			float min_height, float max_height,
			float min_temperature, float max_temperature,
			float min_humidity, float max_humidity,
			const TypedArray<int32_t> &surface_blocks,
			const TypedArray<int32_t> &subsurface_blocks,
			int depth);

	// Extended biome registration with layer blocks
	void add_biome_extended(const String &name,
			float min_height, float max_height,
			float min_temperature, float max_temperature,
			float min_humidity, float max_humidity,
			const TypedArray<int32_t> &surface_blocks,
			const TypedArray<int32_t> &subsurface_blocks,
			int depth,
			int bedrock_block,
			int filler_block);

	void clear_biomes();
	int get_biome_count() const;
	Dictionary get_biome_data(int index) const;

	// Generation methods
	float get_height_at(float x, float z) const;
	Ref<Voxel> get_voxel_at(int x, int y, int z) const;
	int get_biome_index_at(float x, float z) const;

	// Biome blending methods
	Array get_blended_biome_weights(float x, float z) const;
	int select_voxel_with_blending(int x, int y, int z, const Array &biome_weights, bool is_surface) const;
	float get_blended_height_at(float x, float z) const;

	// Utility functions
	float get_temperature_at(float x, float z) const;
	float get_humidity_at(float x, float z) const;

	// Deterministic block selection (replaces randf)
	int select_block_deterministic(int x, int y, int z, const TypedArray<int32_t> &blocks) const;

	// Lazy initialization helper
	void ensure_noise_generators();
};

} // namespace voxel_engine

#endif // BIOME_GENERATOR_H