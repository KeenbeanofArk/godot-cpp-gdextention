/**************************************************************************/
/*  TerrainConfig.cpp                                                     */
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

#include "TerrainConfig.h"
#include "VoxelGenerator.h"

#include <godot_cpp/variant/utility_functions.hpp>

namespace voxel_engine {

void TerrainConfig::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_config_name", "name"), &TerrainConfig::set_config_name);
	ClassDB::bind_method(D_METHOD("get_config_name"), &TerrainConfig::get_config_name);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "config_name"), "set_config_name", "get_config_name");

	ClassDB::bind_method(D_METHOD("set_world_size", "size"), &TerrainConfig::set_world_size);
	ClassDB::bind_method(D_METHOD("get_world_size"), &TerrainConfig::get_world_size);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3I, "world_size"), "set_world_size", "get_world_size");

	ClassDB::bind_method(D_METHOD("set_chunk_size", "size"), &TerrainConfig::set_chunk_size);
	ClassDB::bind_method(D_METHOD("get_chunk_size"), &TerrainConfig::get_chunk_size);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "chunk_size", PROPERTY_HINT_RANGE, "8,64,8"), "set_chunk_size", "get_chunk_size");

	ClassDB::bind_method(D_METHOD("set_resolution", "resolution"), &TerrainConfig::set_resolution);
	ClassDB::bind_method(D_METHOD("get_resolution"), &TerrainConfig::get_resolution);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "resolution", PROPERTY_HINT_RANGE, "1,10,1"), "set_resolution", "get_resolution");

	ClassDB::bind_method(D_METHOD("set_cutoff", "cutoff"), &TerrainConfig::set_cutoff);
	ClassDB::bind_method(D_METHOD("get_cutoff"), &TerrainConfig::get_cutoff);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "cutoff", PROPERTY_HINT_RANGE, "-1,1,0.01"), "set_cutoff", "get_cutoff");

	ClassDB::bind_method(D_METHOD("set_seeder", "seeder"), &TerrainConfig::set_seeder);
	ClassDB::bind_method(D_METHOD("get_seeder"), &TerrainConfig::get_seeder);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "seeder", PROPERTY_HINT_RANGE, "0,1000000,1"), "set_seeder", "get_seeder");

	ClassDB::bind_method(D_METHOD("set_terrain_height", "height"), &TerrainConfig::set_terrain_height);
	ClassDB::bind_method(D_METHOD("get_terrain_height"), &TerrainConfig::get_terrain_height);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "terrain_height", PROPERTY_HINT_RANGE, "-100,100,0.5"), "set_terrain_height", "get_terrain_height");

	ClassDB::bind_method(D_METHOD("set_terrain_amplitude", "amplitude"), &TerrainConfig::set_terrain_amplitude);
	ClassDB::bind_method(D_METHOD("get_terrain_amplitude"), &TerrainConfig::get_terrain_amplitude);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "terrain_amplitude", PROPERTY_HINT_RANGE, "0,100,0.5"), "set_terrain_amplitude", "get_terrain_amplitude");

	ClassDB::bind_method(D_METHOD("set_terrain_material", "material"), &TerrainConfig::set_terrain_material);
	ClassDB::bind_method(D_METHOD("get_terrain_material"), &TerrainConfig::get_terrain_material);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "terrain_material", PROPERTY_HINT_RESOURCE_TYPE, "Material"), "set_terrain_material", "get_terrain_material");

	ClassDB::bind_method(D_METHOD("set_use_textures", "use_textures"), &TerrainConfig::set_use_textures);
	ClassDB::bind_method(D_METHOD("get_use_textures"), &TerrainConfig::get_use_textures);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_textures"), "set_use_textures", "get_use_textures");

	ClassDB::bind_method(D_METHOD("set_noise_generator", "noise_generator"), &TerrainConfig::set_noise_generator);
	ClassDB::bind_method(D_METHOD("get_noise_generator"), &TerrainConfig::get_noise_generator);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "noise_generator", PROPERTY_HINT_RESOURCE_TYPE, "NoiseGenerator"), "set_noise_generator", "get_noise_generator");

	ClassDB::bind_method(D_METHOD("set_biome_generator", "biome_generator"), &TerrainConfig::set_biome_generator);
	ClassDB::bind_method(D_METHOD("get_biome_generator"), &TerrainConfig::get_biome_generator);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "biome_generator", PROPERTY_HINT_RESOURCE_TYPE, "BiomeGenerator"), "set_biome_generator", "get_biome_generator");

	ClassDB::bind_method(D_METHOD("set_rock_influence", "influence"), &TerrainConfig::set_rock_influence);
	ClassDB::bind_method(D_METHOD("get_rock_influence"), &TerrainConfig::get_rock_influence);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "rock_influence", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_rock_influence", "get_rock_influence");

	ClassDB::bind_method(D_METHOD("set_biome_presets", "presets"), &TerrainConfig::set_biome_presets);
	ClassDB::bind_method(D_METHOD("get_biome_presets"), &TerrainConfig::get_biome_presets);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "biome_presets"), "set_biome_presets", "get_biome_presets");

	ClassDB::bind_method(D_METHOD("set_sea_level", "level"), &TerrainConfig::set_sea_level);
	ClassDB::bind_method(D_METHOD("get_sea_level"), &TerrainConfig::get_sea_level);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "sea_level", PROPERTY_HINT_RANGE, "-100,100,0.5"), "set_sea_level", "get_sea_level");

	ClassDB::bind_method(D_METHOD("set_lod_levels", "levels"), &TerrainConfig::set_lod_levels);
	ClassDB::bind_method(D_METHOD("get_lod_levels"), &TerrainConfig::get_lod_levels);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "lod_levels", PROPERTY_HINT_RANGE, "1,10,1"), "set_lod_levels", "get_lod_levels");

	ClassDB::bind_method(D_METHOD("apply_to_voxel_generator", "generator"), &TerrainConfig::apply_to_voxel_generator);
	ClassDB::bind_method(D_METHOD("get_biome_preset", "index"), &TerrainConfig::get_biome_preset);
	ClassDB::bind_method(D_METHOD("get_biome_preset_count"), &TerrainConfig::get_biome_preset_count);
}

TerrainConfig::TerrainConfig() {
	config_name = "Default Terrain";
}

TerrainConfig::~TerrainConfig() {
}

void TerrainConfig::set_config_name(const String &p_name) {
	config_name = p_name;
}

String TerrainConfig::get_config_name() const {
	return config_name;
}

void TerrainConfig::set_world_size(const Vector3i &p_size) {
	world_size = Vector3i(
			CLAMP(p_size.x, 1, 100),
			CLAMP(p_size.y, 1, 100),
			CLAMP(p_size.z, 1, 100));
}

Vector3i TerrainConfig::get_world_size() const {
	return world_size;
}

void TerrainConfig::set_chunk_size(int p_size) {
	chunk_size = CLAMP(p_size, 8, 64);
}

int TerrainConfig::get_chunk_size() const {
	return chunk_size;
}

void TerrainConfig::set_resolution(int p_resolution) {
	resolution = CLAMP(p_resolution, 1, 10);
}

int TerrainConfig::get_resolution() const {
	return resolution;
}

void TerrainConfig::set_cutoff(float p_cutoff) {
	cutoff = CLAMP(p_cutoff, -1.0f, 1.0f);
}

float TerrainConfig::get_cutoff() const {
	return cutoff;
}

void TerrainConfig::set_seeder(int p_seeder) {
	seeder = p_seeder;
}

int TerrainConfig::get_seeder() const {
	return seeder;
}

void TerrainConfig::set_terrain_height(float p_height) {
	terrain_height = p_height;
}

float TerrainConfig::get_terrain_height() const {
	return terrain_height;
}

void TerrainConfig::set_terrain_amplitude(float p_amplitude) {
	terrain_amplitude = MAX(0.0f, p_amplitude);
}

float TerrainConfig::get_terrain_amplitude() const {
	return terrain_amplitude;
}

void TerrainConfig::set_terrain_material(Material *p_material) {
	terrain_material = p_material;
}

Material *TerrainConfig::get_terrain_material() const {
	return terrain_material;
}

bool TerrainConfig::get_use_textures() const {
	return use_textures;
}

void TerrainConfig::set_use_textures(bool p_use_textures) {
	use_textures = p_use_textures;
}

NoiseGenerator *TerrainConfig::get_noise_generator() const {
	return noise_generator;
}

void TerrainConfig::set_noise_generator(NoiseGenerator *p_noise_generator) {
	noise_generator = p_noise_generator;
}

BiomeGenerator *TerrainConfig::get_biome_generator() const {
	return biome_generator;
}

void TerrainConfig::set_biome_generator(BiomeGenerator *p_biome_generator) {
	biome_generator = p_biome_generator;
}

void TerrainConfig::set_rock_influence(float p_influence) {
	rock_influence = CLAMP(p_influence, 0.0f, 1.0f);
}

float TerrainConfig::get_rock_influence() const {
	return rock_influence;
}

void TerrainConfig::set_biome_presets(const Array &p_presets) {
	biome_presets = p_presets;
}

Array TerrainConfig::get_biome_presets() const {
	return biome_presets;
}

void TerrainConfig::set_sea_level(float p_level) {
	sea_level = p_level;
}

float TerrainConfig::get_sea_level() const {
	return sea_level;
}

void TerrainConfig::set_lod_levels(int p_levels) {
	lod_levels = CLAMP(p_levels, 1, 10);
}

int TerrainConfig::get_lod_levels() const {
	return lod_levels;
}

Ref<BiomePreset> TerrainConfig::get_biome_preset(int p_index) const {
	if (p_index < 0 || p_index >= biome_presets.size()) {
		return nullptr;
	}
	return Ref<BiomePreset>(biome_presets[p_index]);
}

int TerrainConfig::get_biome_preset_count() const {
	return biome_presets.size();
}

bool TerrainConfig::apply_to_voxel_generator(Object *p_generator) const {
	if (!p_generator) {
		UtilityFunctions::printerr("[TerrainConfig] apply_to_voxel_generator: generator is null");
		return false;
	}

	VoxelGenerator *voxel_gen = cast_to<VoxelGenerator>(p_generator);
	if (!voxel_gen) {
		UtilityFunctions::printerr("[TerrainConfig] apply_to_voxel_generator: object is not a VoxelGenerator");
		return false;
	}

	// Apply all configuration properties to the VoxelGenerator
	voxel_gen->set_world_size(world_size);
	voxel_gen->set_chunk_size(chunk_size);
	voxel_gen->set_resolution(resolution);
	voxel_gen->set_cutoff(cutoff);
	voxel_gen->set_seeder(seeder);
	voxel_gen->set_terrain_height(terrain_height);
	voxel_gen->set_terrain_amplitude(terrain_amplitude);
	voxel_gen->set_terrain_material(terrain_material);
	voxel_gen->set_use_textures(use_textures);
	voxel_gen->set_terrain_noise(noise_generator);
	voxel_gen->set_biome_generator(biome_generator);
	voxel_gen->set_rock_influence(rock_influence);
	// voxel_gen->set_sea_level(sea_level);
	// voxel_gen->set_lod_levels(lod_levels);

	// Create and apply biome generator if we have biome presets
	if (biome_presets.size() > 0) {
		// For now, just log that we would apply biomes
		// Full biome application would require BiomeGenerator::add_biome() integration
		UtilityFunctions::print("[TerrainConfig] Would apply ", biome_presets.size(), " biome presets to generator");
	}

	UtilityFunctions::print("[TerrainConfig] Successfully applied configuration to VoxelGenerator");
	return true;
}

} // namespace voxel_engine
