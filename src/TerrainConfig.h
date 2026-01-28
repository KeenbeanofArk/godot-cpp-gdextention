/**************************************************************************/
/*  TerrainConfig.h                                                       */
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

#ifndef TERRAIN_CONFIG_H
#define TERRAIN_CONFIG_H

#include "BiomePreset.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/vector3i.hpp>

using namespace godot;

namespace voxel_engine {

// Forward declarations
class VoxelGenerator;
class BiomeGenerator;
class NoiseGenerator;

/// @brief TerrainConfig - Resource class for complete terrain configuration
/// Holds all terrain parameters and can apply them to a VoxelGenerator instance
/// Serializes to .tres files for easy terrain preset management
class TerrainConfig : public Resource {
	GDCLASS(TerrainConfig, Resource);

private:
	String config_name;
	Vector3i world_size = Vector3i(5, 5, 5);
	int chunk_size = 8;
	int resolution = 2;
	float cutoff = 0.1f;
	int seeder = 1240;
	float terrain_height = 4.0f;
	float terrain_amplitude = 8.0f;
	Ref<Material> terrain_material;
	bool use_textures = true;
	Ref<NoiseGenerator> noise_generator;
	Ref<BiomeGenerator> biome_generator;
	float rock_influence = 0.3f;
	Array biome_presets; // Array of BiomePreset resources
	float sea_level = 0.0f;
	int lod_levels = 6;

protected:
	static void _bind_methods();

public:
	TerrainConfig();
	~TerrainConfig();

	// Config name
	void set_config_name(const String &p_name);
	String get_config_name() const;

	// World size
	void set_world_size(const Vector3i &p_size);
	Vector3i get_world_size() const;

	// Chunk size
	void set_chunk_size(int p_size);
	int get_chunk_size() const;

	// Resolution
	void set_resolution(int p_resolution);
	int get_resolution() const;

	// Cutoff
	void set_cutoff(float p_cutoff);
	float get_cutoff() const;

	// Seeder
	void set_seeder(int p_seeder);
	int get_seeder() const;

	// Terrain height
	void set_terrain_height(float p_height);
	float get_terrain_height() const;

	// Terrain amplitude
	void set_terrain_amplitude(float p_amplitude);
	float get_terrain_amplitude() const;

	Ref<Material> get_terrain_material() const;
	void set_terrain_material(const Ref<Material> &p_material);

	bool get_use_textures() const;
	void set_use_textures(bool p_use_textures);

	Ref<NoiseGenerator> get_noise_generator() const;
	void set_noise_generator(const Ref<NoiseGenerator> &p_noise_generator);

	Ref<BiomeGenerator> get_biome_generator() const;
	void set_biome_generator(const Ref<BiomeGenerator> &p_biome_generator);

	// Rock influence
	void set_rock_influence(float p_influence);
	float get_rock_influence() const;

	// Biome presets array
	void set_biome_presets(const Array &p_presets);
	Array get_biome_presets() const;

	// Sea level
	void set_sea_level(float p_level);
	float get_sea_level() const;
	
	// LOD levels
	void set_lod_levels(int p_levels);
	int get_lod_levels() const;

	// Apply this configuration to a VoxelGenerator instance
	// Returns true on success, false on failure
	bool apply_to_voxel_generator(Object *p_generator) const;

	// Helper method to get a biome preset by index
	Ref<BiomePreset> get_biome_preset(int p_index) const;

	// Get count of biome presets
	int get_biome_preset_count() const;
};

} // namespace voxel_engine

#endif // TERRAIN_CONFIG_H
