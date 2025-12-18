/**************************************************************************/
/*  VoxelGenerator.cpp                                                    */
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

#include "VoxelGenerator.h"
#include "Constants.h"
#include "core/voxel_constants.h"
#include "generators/BiomeGenerator.h"
#include "generators/FeatureGenerator.h"
#include "generators/NoiseGenerator.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>
#include <vector>

// Godot includes
#include <godot_cpp/classes/fast_noise_lite.hpp>
#include <godot_cpp/classes/immediate_mesh.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/surface_tool.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/vector3.hpp>

namespace voxel_engine {

void VoxelGenerator::_bind_methods() {
	ClassDB::bind_method(D_METHOD("generate"), &VoxelGenerator::generate);
	ClassDB::bind_method(D_METHOD("generate_async"), &VoxelGenerator::generate_async);
	ClassDB::bind_method(D_METHOD("cancel_generation"), &VoxelGenerator::cancel_generation);
	ClassDB::bind_method(D_METHOD("is_generating"), &VoxelGenerator::is_generating);

	// Async generation property bindings
	ClassDB::bind_method(D_METHOD("set_max_chunks_per_frame", "value"), &VoxelGenerator::set_max_chunks_per_frame);
	ClassDB::bind_method(D_METHOD("get_max_chunks_per_frame"), &VoxelGenerator::get_max_chunks_per_frame);
	ClassDB::bind_method(D_METHOD("set_signal_every_n_chunks", "value"), &VoxelGenerator::set_signal_every_n_chunks);
	ClassDB::bind_method(D_METHOD("get_signal_every_n_chunks"), &VoxelGenerator::get_signal_every_n_chunks);

	ClassDB::bind_method(D_METHOD("set_world_size", "value"), &VoxelGenerator::set_world_size);
	ClassDB::bind_method(D_METHOD("get_world_size"), &VoxelGenerator::get_world_size);
	ClassDB::bind_method(D_METHOD("set_chunk_size", "value"), &VoxelGenerator::set_chunk_size);
	ClassDB::bind_method(D_METHOD("get_chunk_size"), &VoxelGenerator::get_chunk_size);

	// Generation mode and LOD bindings
	ClassDB::bind_method(D_METHOD("set_generation_mode", "value"), &VoxelGenerator::set_generation_mode);
	ClassDB::bind_method(D_METHOD("get_generation_mode"), &VoxelGenerator::get_generation_mode);
	ClassDB::bind_method(D_METHOD("set_surface_band", "value"), &VoxelGenerator::set_surface_band);
	ClassDB::bind_method(D_METHOD("get_surface_band"), &VoxelGenerator::get_surface_band);
	ClassDB::bind_method(D_METHOD("set_lod_level", "value"), &VoxelGenerator::set_lod_level);
	ClassDB::bind_method(D_METHOD("get_lod_level"), &VoxelGenerator::get_lod_level);
	ClassDB::bind_method(D_METHOD("set_heightmap_vertex_limit", "value"), &VoxelGenerator::set_heightmap_vertex_limit);
	ClassDB::bind_method(D_METHOD("get_heightmap_vertex_limit"), &VoxelGenerator::get_heightmap_vertex_limit);

	// Distance-based LOD bindings
	ClassDB::bind_method(D_METHOD("set_enable_distance_lod", "value"), &VoxelGenerator::set_enable_distance_lod);
	ClassDB::bind_method(D_METHOD("get_enable_distance_lod"), &VoxelGenerator::get_enable_distance_lod);
	ClassDB::bind_method(D_METHOD("set_lod_reference_position", "value"), &VoxelGenerator::set_lod_reference_position);
	ClassDB::bind_method(D_METHOD("get_lod_reference_position"), &VoxelGenerator::get_lod_reference_position);
	ClassDB::bind_method(D_METHOD("set_lod_distances", "value"), &VoxelGenerator::set_lod_distances);
	ClassDB::bind_method(D_METHOD("get_lod_distances"), &VoxelGenerator::get_lod_distances);
	ClassDB::bind_method(D_METHOD("reset_lod_distances_to_default"), &VoxelGenerator::reset_lod_distances_to_default);
	ClassDB::bind_method(D_METHOD("calculate_chunk_lod", "chunk_index"), &VoxelGenerator::calculate_chunk_lod);
	ClassDB::bind_method(D_METHOD("get_chunk_center_world_position", "chunk_index"), &VoxelGenerator::get_chunk_center_world_position);
	ClassDB::bind_method(D_METHOD("update_chunks_lod"), &VoxelGenerator::update_chunks_lod);
	ClassDB::bind_method(D_METHOD("set_show_lod_colors", "value"), &VoxelGenerator::set_show_lod_colors);
	ClassDB::bind_method(D_METHOD("get_show_lod_colors"), &VoxelGenerator::get_show_lod_colors);
	ClassDB::bind_method(D_METHOD("set_lod_distance_multiplier", "value"), &VoxelGenerator::set_lod_distance_multiplier);
	ClassDB::bind_method(D_METHOD("get_lod_distance_multiplier"), &VoxelGenerator::get_lod_distance_multiplier);
	ClassDB::bind_method(D_METHOD("set_use_textures", "value"), &VoxelGenerator::set_use_textures);
	ClassDB::bind_method(D_METHOD("get_use_textures"), &VoxelGenerator::get_use_textures);
	ClassDB::bind_method(D_METHOD("set_terrain_material", "material"), &VoxelGenerator::set_terrain_material);
	ClassDB::bind_method(D_METHOD("get_terrain_material"), &VoxelGenerator::get_terrain_material);

	// Terrain noise bindings
	ClassDB::bind_method(D_METHOD("set_terrain_noise", "noise"), &VoxelGenerator::set_terrain_noise);
	ClassDB::bind_method(D_METHOD("get_terrain_noise"), &VoxelGenerator::get_terrain_noise);
	ClassDB::bind_method(D_METHOD("set_detail_noise", "noise"), &VoxelGenerator::set_detail_noise);
	ClassDB::bind_method(D_METHOD("get_detail_noise"), &VoxelGenerator::get_detail_noise);
	ClassDB::bind_method(D_METHOD("set_rock_influence", "value"), &VoxelGenerator::set_rock_influence);
	ClassDB::bind_method(D_METHOD("get_rock_influence"), &VoxelGenerator::get_rock_influence);
	ClassDB::bind_method(D_METHOD("set_terrain_height", "value"), &VoxelGenerator::set_terrain_height);
	ClassDB::bind_method(D_METHOD("get_terrain_height"), &VoxelGenerator::get_terrain_height);
	ClassDB::bind_method(D_METHOD("set_terrain_amplitude", "value"), &VoxelGenerator::set_terrain_amplitude);
	ClassDB::bind_method(D_METHOD("get_terrain_amplitude"), &VoxelGenerator::get_terrain_amplitude);

	// Biome and Feature generator bindings
	ClassDB::bind_method(D_METHOD("set_biome_generator", "generator"), &VoxelGenerator::set_biome_generator);
	ClassDB::bind_method(D_METHOD("get_biome_generator"), &VoxelGenerator::get_biome_generator);
	ClassDB::bind_method(D_METHOD("set_feature_generator", "generator"), &VoxelGenerator::set_feature_generator);
	ClassDB::bind_method(D_METHOD("get_feature_generator"), &VoxelGenerator::get_feature_generator);

	ClassDB::bind_method(D_METHOD("set_resolution", "value"), &VoxelGenerator::set_resolution);
	ClassDB::bind_method(D_METHOD("get_resolution"), &VoxelGenerator::get_resolution);
	ClassDB::bind_method(D_METHOD("set_cutoff", "value"), &VoxelGenerator::set_cutoff);
	ClassDB::bind_method(D_METHOD("get_cutoff"), &VoxelGenerator::get_cutoff);
	ClassDB::bind_method(D_METHOD("set_randomizer", "value"), &VoxelGenerator::set_randomizer);
	ClassDB::bind_method(D_METHOD("get_randomizer"), &VoxelGenerator::get_randomizer);
	ClassDB::bind_method(D_METHOD("set_show_centers", "value"), &VoxelGenerator::set_show_centers);
	ClassDB::bind_method(D_METHOD("get_show_centers"), &VoxelGenerator::get_show_centers);
	ClassDB::bind_method(D_METHOD("set_show_voxel_grid", "value"), &VoxelGenerator::set_show_voxel_grid);
	ClassDB::bind_method(D_METHOD("get_show_voxel_grid"), &VoxelGenerator::get_show_voxel_grid);
	ClassDB::bind_method(D_METHOD("set_show_chunk_grid", "value"), &VoxelGenerator::set_show_chunk_grid);
	ClassDB::bind_method(D_METHOD("get_show_chunk_grid"), &VoxelGenerator::get_show_chunk_grid);
	ClassDB::bind_method(D_METHOD("set_auto_generate", "value"), &VoxelGenerator::set_auto_generate);
	ClassDB::bind_method(D_METHOD("get_auto_generate"), &VoxelGenerator::get_auto_generate);
	ClassDB::bind_method(D_METHOD("set_seeder", "value"), &VoxelGenerator::set_seeder);
	ClassDB::bind_method(D_METHOD("get_seeder"), &VoxelGenerator::get_seeder);
	ClassDB::bind_method(D_METHOD("get_vertex_limit"), &VoxelGenerator::get_vertex_limit);
	ClassDB::bind_method(D_METHOD("set_vertex_limit", "value"), &VoxelGenerator::set_vertex_limit);

	// Bind debug methods
	ClassDB::bind_method(D_METHOD("set_debug_mode", "enabled"), &VoxelGenerator::set_debug_mode);
	ClassDB::bind_method(D_METHOD("get_debug_mode"), &VoxelGenerator::get_debug_mode);

	ClassDB::bind_method(D_METHOD("set_visualize_noise_values", "enabled"), &VoxelGenerator::set_visualize_noise_values);
	ClassDB::bind_method(D_METHOD("get_visualize_noise_values"), &VoxelGenerator::get_visualize_noise_values);

	ClassDB::bind_method(D_METHOD("set_debug_verbosity", "level"), &VoxelGenerator::set_debug_verbosity);
	ClassDB::bind_method(D_METHOD("get_debug_verbosity"), &VoxelGenerator::get_debug_verbosity);

	ClassDB::bind_method(D_METHOD("debug_print_state"), &VoxelGenerator::debug_print_state);
	ClassDB::bind_method(D_METHOD("debug_draw_noise_slice", "y_level"), &VoxelGenerator::debug_draw_noise_slice);
	ClassDB::bind_method(D_METHOD("log_message", "message", "verbosity_level"), &VoxelGenerator::log_message, DEFVAL(1));

	// Heightmap methods
	ClassDB::bind_method(D_METHOD("get_height_at", "fx", "fz"), &VoxelGenerator::get_height_at);

	ClassDB::bind_method(D_METHOD("reset"), &VoxelGenerator::reset);

	// Bind dirty flag / incremental update methods
	ClassDB::bind_method(D_METHOD("mark_chunk_dirty", "chunk_coord"), &VoxelGenerator::mark_chunk_dirty);
	ClassDB::bind_method(D_METHOD("mark_all_chunks_dirty"), &VoxelGenerator::mark_all_chunks_dirty);
	ClassDB::bind_method(D_METHOD("is_chunk_dirty", "chunk_coord"), &VoxelGenerator::is_chunk_dirty);
	ClassDB::bind_method(D_METHOD("regenerate_dirty_chunks"), &VoxelGenerator::regenerate_dirty_chunks);
	ClassDB::bind_method(D_METHOD("invalidate_density_region", "min_voxel", "max_voxel"), &VoxelGenerator::invalidate_density_region);

	// Bind terraforming methods
	ClassDB::bind_method(D_METHOD("modify_terrain", "center", "radius", "delta"), &VoxelGenerator::modify_terrain);
	ClassDB::bind_method(D_METHOD("dig_sphere", "center", "radius", "strength"), &VoxelGenerator::dig_sphere, DEFVAL(1.0f));
	ClassDB::bind_method(D_METHOD("build_sphere", "center", "radius", "strength"), &VoxelGenerator::build_sphere, DEFVAL(1.0f));
	ClassDB::bind_method(D_METHOD("clear_terrain_edits"), &VoxelGenerator::clear_terrain_edits);
	ClassDB::bind_method(D_METHOD("get_terrain_edit_count"), &VoxelGenerator::get_terrain_edit_count);
	ClassDB::bind_method(D_METHOD("get_terrain_edits_data"), &VoxelGenerator::get_terrain_edits_data);
	ClassDB::bind_method(D_METHOD("set_terrain_edits_data", "data"), &VoxelGenerator::set_terrain_edits_data);

	ClassDB::bind_method(D_METHOD("is_object_binding_set_by_parent_constructor"), &VoxelGenerator::is_object_binding_set_by_parent_constructor);

	// Async generation signals
	ADD_SIGNAL(MethodInfo("chunk_ready", PropertyInfo(Variant::INT, "chunk_index"), PropertyInfo(Variant::VECTOR3I, "chunk_coord")));
	ADD_SIGNAL(MethodInfo("generation_progress", PropertyInfo(Variant::INT, "completed"), PropertyInfo(Variant::INT, "total")));
	ADD_SIGNAL(MethodInfo("generation_complete"));

	ADD_GROUP("voxel_generator", "voxel_generator_");
	ADD_SUBGROUP("voxel_generator_", "World Settings");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3I, "world_size", PROPERTY_HINT_RANGE, "1,100,1"), "set_world_size", "get_world_size");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_generate"), "set_auto_generate", "get_auto_generate");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "chunk_size", PROPERTY_HINT_RANGE, "8,64,8"), "set_chunk_size", "get_chunk_size");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "resolution", PROPERTY_HINT_RANGE, "1,10,1"), "set_resolution", "get_resolution");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "cutoff", PROPERTY_HINT_RANGE, "-1,1,0.01"), "set_cutoff", "get_cutoff");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "seeder", PROPERTY_HINT_RANGE, "0,1000000,1"), "set_seeder", "get_seeder");

	ADD_GROUP("Generation Mode", "generation_");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "generation_mode", PROPERTY_HINT_ENUM, "Voxels First,Heightmap First"), "set_generation_mode", "get_generation_mode");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "surface_band", PROPERTY_HINT_RANGE, "1.0,20.0,0.5"), "set_surface_band", "get_surface_band");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "lod_level", PROPERTY_HINT_RANGE, "0,7,1"), "set_lod_level", "get_lod_level");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "heightmap_vertex_limit", PROPERTY_HINT_RANGE, "1000000,268435456,1000000"), "set_heightmap_vertex_limit", "get_heightmap_vertex_limit");

	ADD_GROUP("Distance LOD", "lod_");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enable_distance_lod"), "set_enable_distance_lod", "get_enable_distance_lod");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "lod_reference_position"), "set_lod_reference_position", "get_lod_reference_position");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_FLOAT64_ARRAY, "lod_distances"), "set_lod_distances", "get_lod_distances");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "lod_distance_multiplier", PROPERTY_HINT_RANGE, "1.0,10.0,0.1"), "set_lod_distance_multiplier", "get_lod_distance_multiplier");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_lod_colors"), "set_show_lod_colors", "get_show_lod_colors");

	ADD_GROUP("Rendering", "render_");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_textures"), "set_use_textures", "get_use_textures");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "terrain_material", PROPERTY_HINT_RESOURCE_TYPE, "Material"), "set_terrain_material", "get_terrain_material");

	ADD_GROUP("Terrain Settings", "terrain_");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "terrain_noise", PROPERTY_HINT_RESOURCE_TYPE, "NoiseGenerator"), "set_terrain_noise", "get_terrain_noise");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "detail_noise", PROPERTY_HINT_RESOURCE_TYPE, "NoiseGenerator"), "set_detail_noise", "get_detail_noise");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "terrain_height", PROPERTY_HINT_RANGE, "-100,100,0.5"), "set_terrain_height", "get_terrain_height");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "terrain_amplitude", PROPERTY_HINT_RANGE, "0,100,0.5"), "set_terrain_amplitude", "get_terrain_amplitude");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "rock_influence", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_rock_influence", "get_rock_influence");

	ADD_GROUP("Generators", "");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "biome_generator", PROPERTY_HINT_RESOURCE_TYPE, "BiomeGenerator"), "set_biome_generator", "get_biome_generator");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "feature_generator", PROPERTY_HINT_RESOURCE_TYPE, "FeatureGenerator"), "set_feature_generator", "get_feature_generator");

	ADD_GROUP("Display Settings", "");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "randomizer"), "set_randomizer", "get_randomizer");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_centers"), "set_show_centers", "get_show_centers");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_voxel_grid"), "set_show_voxel_grid", "get_show_voxel_grid");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_chunk_grid"), "set_show_chunk_grid", "get_show_chunk_grid");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "vertex_limit"), "set_vertex_limit", "get_vertex_limit");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "visualize_noise_values"), "set_visualize_noise_values", "get_visualize_noise_values");

	ADD_GROUP("Debug Settings", "debug_");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "debug_mode"), "set_debug_mode", "get_debug_mode");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "debug_verbosity", PROPERTY_HINT_RANGE, "0,3,1"), "set_debug_verbosity", "get_debug_verbosity");

	ADD_GROUP("Async Generation", "async_");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_chunks_per_frame", PROPERTY_HINT_RANGE, "1,32,1"), "set_max_chunks_per_frame", "get_max_chunks_per_frame");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "signal_every_n_chunks", PROPERTY_HINT_RANGE, "1,64,1"), "set_signal_every_n_chunks", "get_signal_every_n_chunks");
}

bool VoxelGenerator::has_object_instance_binding() const {
	return internal::gdextension_interface_object_get_instance_binding(_owner, internal::token, nullptr);
}

VoxelGenerator::VoxelGenerator() :
		object_instance_binding_set_by_parent_constructor(false) {
	// Initialize default values only - don't call Godot API functions yet
	resolution = 1;
	cutoff = 0.1f;
	show_centers = false;
	show_voxel_grid = false;
	show_chunk_grid = false;
	randomizer = false;
	seeder = 1240;
	auto_generate = false;
	vertex_limit = false;

	// Terrain generation defaults
	rock_influence = 0.3f;
	terrain_height = 4.0f;
	terrain_amplitude = 8.0f;

	// Distance-based LOD defaults
	enable_distance_lod = false;
	lod_reference_position = Vector3(0, 0, 0);
	lod_distances_custom = false;
	// Initialize default LOD distances based on chunk_size
	lod_distances.resize(8);
	for (int i = 0; i < 8; i++) {
		lod_distances[i] = static_cast<double>(chunk_size) * static_cast<double>(1 << (i + 1));
	}

	// Note: terrain_noise and detail_noise are created lazily in generate()
	// to avoid Godot's "Instantiated X used as default value" warning
}

VoxelGenerator::~VoxelGenerator() {
	// Cancel any running async generation and wait for completion
	if (async_task_group != 0) {
		cancel_requested.store(true);
		WorkerThreadPool::get_singleton()->wait_for_group_task_completion(async_task_group);
		async_task_group = 0;
	}

	// Clear pending mesh queue
	clear_pending_meshes();

	// Clean up chunks if they exist
	for (Chunk *chunk : chunks) {
		if (chunk && is_instance_valid(chunk)) {
			chunk->queue_free(); // Use queue_free() instead of memdelete
		}
	}
	chunks.clear();
	log_message("VoxelGenerator destroyed and chunks cleaned up.", 1);
}

void VoxelGenerator::calculate_world_size() {
	// Historically this method derived world_size from gen_size.
	// For Option A semantics we treat `world_size` as chunk counts set by the user,
	// so do not override `world_size` here.
	log_message(String("calculate_world_size() called; keeping user-set world_size: {0}").format(Array::make(world_size)), 1);
}

void VoxelGenerator::recalculate_voxel_scale() {
	// Voxel size is divided by resolution to create finer marching cubes samples
	// Higher resolution = smaller voxels = more triangles within same physical space

	// Base voxel size from raw resolution setting
	float base_scale = 1.0f / static_cast<float>(std::max(1, resolution));
	base_voxel_size = Vector3(base_scale, base_scale, base_scale);

	// Effective voxel size accounts for LOD level
	int eff_resolution = get_effective_resolution();
	float eff_scale = 1.0f / static_cast<float>(std::max(1, eff_resolution));
	effective_voxel_size = Vector3(eff_scale, eff_scale, eff_scale);

	log_message(String("Voxel sizes set - base: {0} (resolution={1}), effective: {2} (eff_resolution={3})")
						.format(Array::make(base_voxel_size, resolution, effective_voxel_size, eff_resolution)),
			2);
}

bool VoxelGenerator::is_object_binding_set_by_parent_constructor() const {
	return object_instance_binding_set_by_parent_constructor;
}

bool VoxelGenerator::is_instance_valid(Chunk *chunk) const {
	// Check if the chunk is valid and not null
	return chunk != nullptr && chunk->is_inside_tree();
}

void VoxelGenerator::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			// Initialize Godot-specific settings now that object is fully ready
			set_name("VoxelGenerator");
			set_process(false); // Will be enabled during async generation
			set_physics_process(false);

			remove_children();
			//randomize_seed();

			if (auto_generate) {
				generate();
				log_message("VoxelGenerator is ready and auto generation is enabled. Voxel grid generated.", 1);
				create_chunks();
			} else {
				log_message("VoxelGenerator is ready, but auto generation is disabled. Call generate() to create the voxel grid.", 1);
			}
			break;
		}
		case NOTIFICATION_PROCESS: {
			// Handle async mesh streaming in _process
			_process(get_process_delta_time());
			break;
		}
		case NOTIFICATION_PREDELETE:
			// Cancel any running async generation
			if (async_task_group != 0) {
				cancel_requested.store(true);
				WorkerThreadPool::get_singleton()->wait_for_group_task_completion(async_task_group);
				async_task_group = 0;
			}
			clear_pending_meshes();

			// Make sure to clean up chunks when the generator is deleted
			for (Chunk *chunk : chunks) {
				if (chunk && is_instance_valid(chunk)) {
					chunk->queue_free(); // Use queue_free() instead of memdelete
				}
			}
			chunks.clear();
			break;
		default:
			break;
	}
};

void VoxelGenerator::reset() {
	// Clean up chunks
	for (Chunk *chunk : chunks) {
		memdelete(chunk); // Use memdelete for cleanup
	}
	chunks.clear();

	remove_children();
	//randomize_seed(); // Do we want to randomize seed on reset?
	resolution = 1;
	cutoff = 0.0f;
	show_centers = false;
	show_voxel_grid = false;
	show_chunk_grid = false;
	seeder = 1240;
	vertex_limit = false;
	rock_influence = 0.3f;
	terrain_height = 4.0f;
	terrain_amplitude = 8.0f;

	if (auto_generate) {
		generate();
	} else {
		log_message("VoxelGenerator reset. Call generate() to create the voxel grid.", 1);
	}
}

void VoxelGenerator::set_chunk_size(int value) {
	// Ensure the value is within allowed range
	if (value >= Constants::MIN_CHUNK_SIZE && value <= Constants::MAX_CHUNK_SIZE) {
		// Only update if value has changed
		if (chunk_size != value) {
			chunk_size = value;
			log_message(String("Chunk size set to: {0}").format(Array::make(chunk_size)), 2);

			// Update default LOD distances if not using custom values
			if (!lod_distances_custom) {
				initialize_default_lod_distances();
			}

			// If auto_generate is enabled, destroy and recreate chunks with new size
			if (auto_generate) {
				create_chunks();
				generate();
			}
		}
	}
}

int VoxelGenerator::get_chunk_size() const {
	return chunk_size;
}

// ============================================================================
// Generation Mode and LOD Getters/Setters
// ============================================================================

void VoxelGenerator::set_generation_mode(int value) {
	GenerationMode new_mode = static_cast<GenerationMode>(CLAMP(value, 0, 1));
	if (generation_mode != new_mode) {
		generation_mode = new_mode;
		log_message(String("Generation mode set to: {0}").format(Array::make(generation_mode == VOXELS_FIRST ? "Voxels First" : "Heightmap First")), 2);
		// Clear heightmap cache when switching modes
		if (generation_mode == VOXELS_FIRST) {
			clear_heightmap_cache();
		}
		if (auto_generate)
			generate();
	}
}

int VoxelGenerator::get_generation_mode() const {
	return static_cast<int>(generation_mode);
}

void VoxelGenerator::set_surface_band(float value) {
	surface_band = CLAMP(value, 1.0f, 20.0f);
	log_message(String("Surface band set to: {0}").format(Array::make(surface_band)), 2);
	if (auto_generate && generation_mode == HEIGHTMAP_FIRST)
		generate();
}

float VoxelGenerator::get_surface_band() const {
	return surface_band;
}

void VoxelGenerator::set_lod_level(int value) {
	int new_lod = CLAMP(value, 0, voxel_engine::MAX_LOD_LEVELS - 1);
	if (lod_level != new_lod) {
		lod_level = new_lod;
		log_message(String("LOD level set to: {0} (effective resolution: {1})").format(Array::make(lod_level, get_effective_resolution())), 2);

		// Recalculate voxel scale since LOD affects effective resolution
		recalculate_voxel_scale();

		// Invalidate caches since effective resolution changed
		clear_density_cache();
		clear_heightmap_cache();

		if (auto_generate)
			generate();
	}
}

int VoxelGenerator::get_lod_level() const {
	return lod_level;
}

void VoxelGenerator::set_heightmap_vertex_limit(int value) {
	heightmap_vertex_limit = CLAMP(value, 1000000, 268435456); // 1M to 256M
	log_message(String("Heightmap vertex limit set to: {0}").format(Array::make(heightmap_vertex_limit)), 2);
}

int VoxelGenerator::get_heightmap_vertex_limit() const {
	return heightmap_vertex_limit;
}

int VoxelGenerator::get_effective_resolution() const {
	// Reduce resolution by powers of 2 based on LOD level
	// LOD 0 = full resolution, LOD 1 = half, LOD 2 = quarter, etc.
	return std::max(1, resolution >> lod_level);
}

float VoxelGenerator::get_effective_surface_band() const {
	// Widen surface band at higher LOD levels to prevent gaps
	return surface_band * (1.0f + lod_level * 0.5f);
}

// ============================================================================
// Distance-Based LOD Implementation
// ============================================================================

void VoxelGenerator::initialize_default_lod_distances() {
	// Set default LOD distances based on chunk_size and multiplier
	// Formula: chunk_size * multiplier^(lod+1) for each LOD level 0-7
	lod_distances.resize(8);
	for (int i = 0; i < 8; i++) {
		lod_distances[i] = static_cast<double>(chunk_size) * std::pow(static_cast<double>(lod_distance_multiplier), i + 1);
	}
	log_message(String("Initialized LOD distances (chunk_size={0}, multiplier={1}): [{2}, {3}, {4}, {5}, {6}, {7}, {8}, {9}]")
						.format(Array::make(chunk_size, lod_distance_multiplier,
								lod_distances[0], lod_distances[1], lod_distances[2], lod_distances[3],
								lod_distances[4], lod_distances[5], lod_distances[6], lod_distances[7])),
			2);
}

int VoxelGenerator::get_effective_resolution_for_lod(int lod) const {
	// Reduce resolution by powers of 2 based on LOD level
	// LOD 0 = full resolution, LOD 1 = half, LOD 2 = quarter, etc.
	return std::max(1, resolution >> lod);
}

void VoxelGenerator::set_enable_distance_lod(bool value) {
	if (enable_distance_lod != value) {
		enable_distance_lod = value;
		log_message(String("Distance-based LOD {0}").format(Array::make(enable_distance_lod ? "enabled" : "disabled")), 2);
		if (enable_distance_lod && lod_distances.size() == 0) {
			initialize_default_lod_distances();
		}
	}
}

bool VoxelGenerator::get_enable_distance_lod() const {
	return enable_distance_lod;
}

void VoxelGenerator::set_lod_reference_position(const Vector3 &value) {
	lod_reference_position = value;
	log_message(String("LOD reference position set to: ({0}, {1}, {2})")
						.format(Array::make(value.x, value.y, value.z)),
			3);
}

Vector3 VoxelGenerator::get_lod_reference_position() const {
	return lod_reference_position;
}

void VoxelGenerator::set_lod_distances(const PackedFloat64Array &value) {
	if (value.size() >= 8) {
		lod_distances = value;
		lod_distances_custom = true;
		log_message(String("Custom LOD distances set: [{0}, {1}, {2}, {3}, {4}, {5}, {6}, {7}]")
							.format(Array::make(
									lod_distances[0], lod_distances[1], lod_distances[2], lod_distances[3],
									lod_distances[4], lod_distances[5], lod_distances[6], lod_distances[7])),
				2);
	} else {
		log_message("LOD distances array must have at least 8 values", 1);
	}
}

PackedFloat64Array VoxelGenerator::get_lod_distances() const {
	return lod_distances;
}

void VoxelGenerator::reset_lod_distances_to_default() {
	lod_distances_custom = false;
	initialize_default_lod_distances();
}

Vector3 VoxelGenerator::get_chunk_center_world_position(int chunk_index) const {
	Vector3i chunk_coord = index_to_chunk_coord(chunk_index);
	if (chunk_coord.x < 0) {
		return Vector3(0, 0, 0); // Invalid index
	}

	// Calculate world extent
	float world_extent_x = static_cast<float>(world_size.x * chunk_size);
	float world_extent_y = static_cast<float>(world_size.y * chunk_size);
	float world_extent_z = static_cast<float>(world_size.z * chunk_size);

	// Calculate chunk center in world space
	// Chunks are centered around origin, so we need to offset
	Vector3 center;
	center.x = (chunk_coord.x + 0.5f) * chunk_size - world_extent_x * 0.5f;
	center.y = (chunk_coord.y + 0.5f) * chunk_size - world_extent_y * 0.5f;
	center.z = (chunk_coord.z + 0.5f) * chunk_size - world_extent_z * 0.5f;

	return center;
}

int VoxelGenerator::calculate_chunk_lod(int chunk_index) const {
	if (!enable_distance_lod || lod_distances.size() < 8) {
		return lod_level; // Fall back to global LOD
	}

	Vector3 chunk_center = get_chunk_center_world_position(chunk_index);
	float distance = chunk_center.distance_to(lod_reference_position);

	// Find appropriate LOD level based on distance thresholds
	for (int lod = 0; lod < 7; lod++) {
		if (distance < lod_distances[lod]) {
			return lod;
		}
	}
	return 7; // Maximum LOD for very distant chunks
}

int VoxelGenerator::update_chunks_lod() {
	if (!enable_distance_lod) {
		log_message("Distance-based LOD is disabled. Enable it first with enable_distance_lod = true", 1);
		return 0;
	}

	int chunks_needing_regen = 0;
	std::lock_guard<std::mutex> lock(chunks_mutex);

	for (int i = 0; i < static_cast<int>(chunks.size()); i++) {
		Chunk *chunk = chunks[i];
		if (!chunk || !is_instance_valid(chunk)) {
			continue;
		}

		int new_lod = calculate_chunk_lod(i);
		int current_lod = chunk->get_current_lod_level();

		if (new_lod != current_lod) {
			chunk->set_current_lod_level(new_lod);
			// Mark as dirty so regenerate_dirty_chunks() will update it
			if (i < static_cast<int>(chunk_dirty_flags.size())) {
				chunk_dirty_flags[i] = true;
			}
			chunks_needing_regen++;
			log_message(String("Chunk {0} LOD changed: {1} -> {2}")
								.format(Array::make(i, current_lod, new_lod)),
					3);
		}
	}

	log_message(String("update_chunks_lod(): {0} chunks need regeneration").format(Array::make(chunks_needing_regen)), 3);
	return chunks_needing_regen;
}

void VoxelGenerator::set_lod_distance_multiplier(float value) {
	float new_multiplier = CLAMP(value, 1.0f, 10.0f);
	if (lod_distance_multiplier != new_multiplier) {
		lod_distance_multiplier = new_multiplier;
		log_message(String("LOD distance multiplier set to: {0}").format(Array::make(lod_distance_multiplier)), 2);
		// Recalculate default distances if not using custom values
		if (!lod_distances_custom) {
			initialize_default_lod_distances();
		}
	}
}

float VoxelGenerator::get_lod_distance_multiplier() const {
	return lod_distance_multiplier;
}

void VoxelGenerator::set_show_lod_colors(bool value) {
	if (show_lod_colors != value) {
		show_lod_colors = value;
		log_message(String("LOD color visualization {0}").format(Array::make(show_lod_colors ? "enabled" : "disabled")), 2);
		// Regenerate to apply color changes
		if (auto_generate && enable_distance_lod) {
			mark_all_chunks_dirty();
			regenerate_dirty_chunks();
		}
	}
}

bool VoxelGenerator::get_show_lod_colors() const {
	return show_lod_colors;
}

void VoxelGenerator::set_use_textures(bool value) {
	if (use_textures == value) {
		return;
	}
	use_textures = value;
	log_message(String("Use textures set to: {0}").format(Array::make(use_textures)), 2);

	Ref<ShaderMaterial> shader_material = terrain_material;
	if (shader_material.is_valid()) {
		shader_material->set_shader_parameter("use_textures", use_textures);
	}
}

bool VoxelGenerator::get_use_textures() const {
	return use_textures;
}

void VoxelGenerator::set_terrain_material(const Ref<Material> &value) {
	terrain_material = value;
	log_message(String("Terrain material set: {0}").format(Array::make(terrain_material.is_valid() ? "valid" : "null")), 2);

	Ref<ShaderMaterial> shader_material = terrain_material;
	if (shader_material.is_valid()) {
		shader_material->set_shader_parameter("use_textures", use_textures);
	}

	// Propagate material to existing chunks (main thread only).
	{
		std::lock_guard<std::mutex> lock(chunks_mutex);
		for (Chunk *chunk : chunks) {
			if (chunk && is_instance_valid(chunk)) {
				chunk->set_terrain_material(terrain_material);
			}
		}
	}
}

Ref<Material> VoxelGenerator::get_terrain_material() const {
	return terrain_material;
}

Color VoxelGenerator::get_lod_color(int lod) const {
	// Map LOD 0-7 to hue: green (0.33) -> yellow (0.16) -> red (0.0)
	// LOD 0 = green (high detail), LOD 7 = red (low detail)
	float hue = 0.33f * (1.0f - static_cast<float>(CLAMP(lod, 0, 7)) / 7.0f);
	float saturation = 0.8f;
	float value = 0.9f;

	// HSV to RGB conversion
	float c = value * saturation;
	float x = c * (1.0f - std::abs(std::fmod(hue * 6.0f, 2.0f) - 1.0f));
	float m = value - c;

	float r, g, b;
	if (hue < 1.0f / 6.0f) {
		r = c;
		g = x;
		b = 0;
	} else if (hue < 2.0f / 6.0f) {
		r = x;
		g = c;
		b = 0;
	} else if (hue < 3.0f / 6.0f) {
		r = 0;
		g = c;
		b = x;
	} else if (hue < 4.0f / 6.0f) {
		r = 0;
		g = x;
		b = c;
	} else if (hue < 5.0f / 6.0f) {
		r = x;
		g = 0;
		b = c;
	} else {
		r = c;
		g = 0;
		b = x;
	}

	return Color(r + m, g + m, b + m);
}

void VoxelGenerator::set_vertex_limit(bool value) {
	vertex_limit = value;
	log_message(String("Vertex limit set to: {0}").format(Array::make(vertex_limit)), 2);
	if (auto_generate)
		generate();
}

bool VoxelGenerator::get_vertex_limit() const {
	return vertex_limit;
}

void VoxelGenerator::set_auto_generate(bool value) {
	auto_generate = value;
	log_message(String("Auto generation set to: {0}").format(Array::make(auto_generate)), 2);
	if (auto_generate)
		generate();
}

bool VoxelGenerator::get_auto_generate() const {
	return auto_generate;
}

void VoxelGenerator::set_world_size(const Vector3i &value) {
	world_size = value;
	log_message(String("World size set to: {0}").format(Array::make(world_size)), 2);
	if (auto_generate)
		generate();
}

Vector3i VoxelGenerator::get_world_size() const {
	return world_size;
}

void VoxelGenerator::set_terrain_noise(const Ref<NoiseGenerator> &p_noise) {
	terrain_noise = p_noise;
	if (terrain_noise.is_valid()) {
		terrain_noise->set_seed(seeder);
	}
	log_message("Terrain noise set", 2);
	if (auto_generate)
		generate();
}

Ref<NoiseGenerator> VoxelGenerator::get_terrain_noise() const {
	return terrain_noise;
}

void VoxelGenerator::set_detail_noise(const Ref<NoiseGenerator> &p_noise) {
	detail_noise = p_noise;
	if (detail_noise.is_valid()) {
		detail_noise->set_seed(seeder + 1);
	}
	log_message("Detail noise set", 2);
	if (auto_generate)
		generate();
}

Ref<NoiseGenerator> VoxelGenerator::get_detail_noise() const {
	return detail_noise;
}

void VoxelGenerator::set_biome_generator(const Ref<BiomeGenerator> &p_generator) {
	biome_generator = p_generator;
	log_message("Biome generator set", 2);
	if (auto_generate)
		generate();
}

Ref<BiomeGenerator> VoxelGenerator::get_biome_generator() const {
	return biome_generator;
}

void VoxelGenerator::set_feature_generator(const Ref<FeatureGenerator> &p_generator) {
	feature_generator = p_generator;
	log_message("Feature generator set", 2);
	if (auto_generate)
		generate();
}

Ref<FeatureGenerator> VoxelGenerator::get_feature_generator() const {
	return feature_generator;
}

void VoxelGenerator::set_rock_influence(float value) {
	rock_influence = CLAMP(value, 0.0f, 1.0f);
	log_message(String("Rock influence set to: {0}").format(Array::make(rock_influence)), 2);
	if (auto_generate)
		generate();
}

float VoxelGenerator::get_rock_influence() const {
	return rock_influence;
}

void VoxelGenerator::set_terrain_height(float value) {
	terrain_height = value;
	log_message(String("Terrain height set to: {0}").format(Array::make(terrain_height)), 2);
	if (auto_generate)
		generate();
}

float VoxelGenerator::get_terrain_height() const {
	return terrain_height;
}

void VoxelGenerator::set_terrain_amplitude(float value) {
	terrain_amplitude = MAX(value, 0.0f);
	log_message(String("Terrain amplitude set to: {0}").format(Array::make(terrain_amplitude)), 2);
	if (auto_generate)
		generate();
}

float VoxelGenerator::get_terrain_amplitude() const {
	return terrain_amplitude;
}

void VoxelGenerator::set_resolution(int value) {
	resolution = std::max(1, value);
	log_message(String("Resolution set to: {0}").format(Array::make(resolution)), 2);

	// Invalidate caches since resolution affects density sampling grid
	clear_density_cache();
	clear_heightmap_cache();

	// Calculate and log estimated sample count for performance awareness
	int base_voxels = std::max(1, world_size.x) * std::max(1, world_size.y) * std::max(1, world_size.z) * chunk_size * chunk_size * chunk_size;
	int64_t total_samples = static_cast<int64_t>(base_voxels) * resolution * resolution * resolution;
	int multiplier = resolution * resolution * resolution;
	log_message(String("Estimated marching cubes samples: {0} ({1}x base)").format(Array::make(total_samples, multiplier)), 1);

	// Performance warning and auto vertex_limit for high resolutions
	// Higher threshold for heightmap mode (processes fewer voxels)
	int auto_enable_threshold = (generation_mode == HEIGHTMAP_FIRST) ? 200 : 100;
	if (resolution >= auto_enable_threshold) {
		UtilityFunctions::push_warning(String("[VoxelGenerator] High resolution ({0}) may cause performance issues. "
											  "Resolution^3 = {1}x more marching cubes samples. Vertex limit auto-enabled.")
						.format(Array::make(resolution, multiplier)));
		if (!vertex_limit) {
			vertex_limit = true;
			log_message(String("Vertex limit auto-enabled due to high resolution (threshold: {0})").format(Array::make(auto_enable_threshold)), 1);
		}
	} else {
		// Auto-disable vertex limit when resolution drops below threshold
		if (vertex_limit) {
			vertex_limit = false;
			log_message(String("Vertex limit auto-disabled (resolution < {0})").format(Array::make(auto_enable_threshold)), 1);
		}
	}

	// Recalculate voxel scale since resolution affects marching cube size
	recalculate_voxel_scale();

	if (auto_generate)
		generate();
}

int VoxelGenerator::get_resolution() const {
	return resolution;
}

void VoxelGenerator::set_cutoff(float value) {
	cutoff = value;
	log_message(String("Cutoff set to: {0}").format(Array::make(cutoff)), 2);
	if (auto_generate)
		generate();
}

float VoxelGenerator::get_cutoff() const {
	return cutoff;
}

void VoxelGenerator::set_randomizer(bool value) {
	randomizer = value;
	if (randomizer == true) {
		randomize_seed();
		log_message(String("Randomizer enabled. New seed: {0}").format(Array::make(seeder)), 2);
		if (auto_generate)
			generate();
	}
}

bool VoxelGenerator::get_randomizer() const {
	return randomizer;
}

void VoxelGenerator::set_show_centers(bool value) {
	show_centers = value;
	log_message(String("Show centers set to: {0}").format(Array::make(show_centers)), 2);
	for (int i = 0; i < get_child_count(); ++i) {
		MeshInstance3D *child = Object::cast_to<MeshInstance3D>(get_child(i));
		if (child && child->get_name() == godot::StringName("MeshInstanceCenters")) {
			child->set_visible(value);
		}
	}
}

bool VoxelGenerator::get_show_centers() const {
	return show_centers;
}

void VoxelGenerator::set_show_voxel_grid(bool value) {
	show_voxel_grid = value;
	log_message(String("Show voxel grid set to: {0}").format(Array::make(show_voxel_grid)), 2);
	for (int i = 0; i < get_child_count(); ++i) {
		MeshInstance3D *child = Object::cast_to<MeshInstance3D>(get_child(i));
		if (child && child->get_name() == StringName("MeshInstanceVoxelGrid")) {
			child->set_visible(value);
		}
	}
}

void VoxelGenerator::set_show_chunk_grid(bool value) {
	show_chunk_grid = value;
	log_message(String("Show chunk grid set to: {0}").format(Array::make(show_chunk_grid)), 2);
	for (int i = 0; i < get_child_count(); ++i) {
		MeshInstance3D *child = Object::cast_to<MeshInstance3D>(get_child(i));
		if (child && child->get_name() == StringName("MeshInstanceChunkGrid")) {
			child->set_visible(value);
		}
	}
}

void VoxelGenerator::set_seeder(int value) {
	seeder = value;
	log_message(String("Seeder set to: {0}").format(Array::make(seeder)), 2);

	// Sync seeds to noise generators
	if (terrain_noise.is_valid()) {
		terrain_noise->set_seed(seeder);
	}
	if (detail_noise.is_valid()) {
		detail_noise->set_seed(seeder + 1); // Offset to avoid identical patterns
	}

	if (auto_generate)
		generate();
}

int VoxelGenerator::get_seeder() const {
	return seeder;
}

bool VoxelGenerator::get_show_voxel_grid() const {
	return show_voxel_grid;
}

bool VoxelGenerator::get_show_chunk_grid() const {
	return show_chunk_grid;
}

void VoxelGenerator::remove_children() {
	// Clear chunks vector first to prevent stale pointers
	chunks.clear();

	while (get_child_count() > 0) {
		Node *child = get_child(0);
		remove_child(child);
		child->queue_free();
	}
}

void VoxelGenerator::randomize_seed() {
	seeder = UtilityFunctions::randi();
	log_message(String("Random seed generated: {0}").format(Array::make(seeder)), 2);

	// Sync seeds to noise generators
	if (terrain_noise.is_valid()) {
		terrain_noise->set_seed(seeder);
	}
	if (detail_noise.is_valid()) {
		detail_noise->set_seed(seeder + 1);
	}
}

void VoxelGenerator::generate() {
	// Prevent concurrent generation
	if (generation_in_progress.exchange(true)) {
		log_message("Generation already in progress, aborting", 1);
		return;
	}

	log_message("VoxelGenerator::generate() called", 2);
	log_message(String("Generation mode: {0}, LOD level: {1}, Effective resolution: {2}")
						.format(Array::make(generation_mode == VOXELS_FIRST ? "Voxels First" : "Heightmap First",
								lod_level, get_effective_resolution())),
			2);
	log_message("Starting voxel generation with:", 3);
	log_message(String("  Terrain Height: {0}, Amplitude: {1}, Rock Influence: {2}, Resolution: {3}, Cutoff: {4}, Seed: {5}")
						.format(Array::make(terrain_height, terrain_amplitude, rock_influence, resolution, cutoff, seeder)),
			3);

	remove_children();

	///////////////////////////////////////////////////////////////////////////////////

	// Ensure noise generators are valid (created lazily to avoid Godot warnings)
	if (!terrain_noise.is_valid()) {
		terrain_noise.instantiate();
		terrain_noise->set_period(50.0f);
		terrain_noise->set_octaves(4);
		terrain_noise->set_persistence(0.5f);
		terrain_noise->set_lacunarity(2.0f);
		terrain_noise->set_seed(seeder);
	}
	if (!detail_noise.is_valid()) {
		detail_noise.instantiate();
		detail_noise->set_period(10.0f);
		detail_noise->set_octaves(3);
		detail_noise->set_persistence(0.6f);
		detail_noise->set_lacunarity(2.5f);
		detail_noise->set_seed(seeder + 1);
	}

	log_message("Noise generators initialized", 2);

	// Ensure voxel scale is current before generation
	ensure_vertical_extent_for_biomes();
	recalculate_voxel_scale();

	// Calculate total chunks AFTER vertical extent expansion
	total_chunks = world_size.x * world_size.y * world_size.z;

	// Initialize dirty flags if needed
	{
		std::lock_guard<std::mutex> lock(chunks_mutex);
		if (chunk_dirty_flags.size() != static_cast<size_t>(total_chunks)) {
			chunk_dirty_flags.resize(total_chunks, false);
		}
	}

	// Create chunk nodes for this generation pass
	create_chunks();

	// Compute procedural biomes/features before density cache sampling
	prepare_procedural_features();

	log_message("Starting per-chunk mesh generation", 2);

	// Build density cache before chunk generation
	if (generation_mode == VOXELS_FIRST) {
		build_density_cache();
	} else {
		build_heightmap_cache();
		build_density_cache();
	}

	// Generate mesh for each chunk synchronously
	for (int chunk_index = 0; chunk_index < total_chunks; ++chunk_index) {
		generate_chunk_mesh_sync(chunk_index);
	}

	log_message(String("Generated {0} chunk meshes").format(Array::make(total_chunks)), 2);

	// Log LOD distribution summary when distance LOD is enabled
	if (enable_distance_lod && debug_mode) {
		int lod_counts[8] = { 0 };
		for (int i = 0; i < total_chunks; ++i) {
			if (i < static_cast<int>(chunks.size()) && chunks[i] && is_instance_valid(chunks[i])) {
				int lod = chunks[i]->get_current_lod_level();
				if (lod >= 0 && lod <= 7) {
					lod_counts[lod]++;
				}
			}
		}
		String summary = "LOD Distribution: ";
		for (int i = 0; i < 8; ++i) {
			if (lod_counts[i] > 0) {
				summary += String("L{0}={1} ").format(Array::make(i, lod_counts[i]));
			}
		}
		log_message(summary, 1);
	}

	// Physical extent for visualizations
	float physical_extent_x = static_cast<float>(std::max(1, world_size.x) * std::max(1, chunk_size));
	float physical_extent_y = static_cast<float>(std::max(1, world_size.y) * std::max(1, chunk_size));
	float physical_extent_z = static_cast<float>(std::max(1, world_size.z) * std::max(1, chunk_size));
	Vector3 physical_extent = Vector3(physical_extent_x, physical_extent_y, physical_extent_z);

	// Use effective resolution for voxel grid
	int eff_resolution = get_effective_resolution();
	Vector3 eff_voxel_size = Vector3(1.0f, 1.0f, 1.0f) / static_cast<float>(std::max(1, eff_resolution));

	// Centers visualization (debug points) - empty for now
	Ref<ImmediateMesh> mesh_centers;
	mesh_centers.instantiate();
	MeshInstance3D *mi_centers = memnew(MeshInstance3D);
	mi_centers->set_name("MeshInstanceCenters");
	mi_centers->set_visible(show_centers);
	mi_centers->set_mesh(mesh_centers);
	add_child(mi_centers);

	// Voxel grid visualization (debug lines) - draws cube wireframes for solid voxels
	// Uses same logic as terrain generation (respects heightmap mode and surface band)
	Ref<ImmediateMesh> mesh_cubes;
	mesh_cubes.instantiate();

	if (show_voxel_grid) {
		mesh_cubes->surface_begin(Mesh::PRIMITIVE_LINES);

		int total_voxels_x = std::max(1, world_size.x) * std::max(1, chunk_size) * eff_resolution;
		int total_voxels_y = std::max(1, world_size.y) * std::max(1, chunk_size) * eff_resolution;
		int total_voxels_z = std::max(1, world_size.z) * std::max(1, chunk_size) * eff_resolution;

		Color grid_color(0.5f, 0.5f, 0.5f, 1.0f); // Gray color for voxel grid

		int cubes_vertex_count = 0;
		int skipped_voxels = 0;

		// Get effective surface band for heightmap mode
		float eff_surface_band = get_effective_surface_band();

		if (generation_mode == HEIGHTMAP_FIRST) {
			// Heightmap-first mode: only process voxels within surface band
			for (int ix = 0; ix < total_voxels_x; ++ix) {
				for (int iz = 0; iz < total_voxels_z; ++iz) {
					// Get terrain height at this X-Z position
					float terrain_height_at_xz = get_height_at(ix, iz);
					float world_y_center = terrain_height_at_xz;

					// Calculate Y range to process (surface band)
					float y_min_world = world_y_center - eff_surface_band;
					float y_max_world = world_y_center + eff_surface_band;

					// Convert to voxel indices
					int iy_min = std::max(0, static_cast<int>((y_min_world + physical_extent.y * 0.5f) / eff_voxel_size.y));
					int iy_max = std::min(total_voxels_y - 1, static_cast<int>((y_max_world + physical_extent.y * 0.5f) / eff_voxel_size.y));

					// Only process voxels within the surface band
					for (int iy = iy_min; iy <= iy_max; ++iy) {
						if (cubes_vertex_count >= Constants::MAX_VERTICES || vertex_limit) {
							break;
						}

						// Calculate center position
						Vector3 center;
						center.x = -physical_extent.x * 0.5f + (ix + 0.5f) * eff_voxel_size.x;
						center.y = -physical_extent.y * 0.5f + (iy + 0.5f) * eff_voxel_size.y;
						center.z = -physical_extent.z * 0.5f + (iz + 0.5f) * eff_voxel_size.z;

						// Check center density - only draw for solid voxels
						float center_value = get_terrain_density(center);
						if (center_value >= cutoff) {
							continue; // Skip air voxels
						}

						// Draw cube wireframe
						Vector<Vector3> cube_vertices = create_cube_vertices(center);
						const int edges[12][2] = {
							{ 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 },
							{ 0, 4 }, { 2, 6 }, { 5, 6 }, { 5, 4 },
							{ 5, 1 }, { 6, 7 }, { 4, 7 }, { 3, 7 }
						};
						for (int e = 0; e < 12; ++e) {
							mesh_cubes->surface_set_color(grid_color);
							mesh_cubes->surface_add_vertex(cube_vertices[edges[e][0]]);
							mesh_cubes->surface_add_vertex(cube_vertices[edges[e][1]]);
						}
						cubes_vertex_count += 24;
					}

					// Count skipped voxels (outside surface band)
					skipped_voxels += (total_voxels_y - (iy_max - iy_min + 1));

					if (cubes_vertex_count >= Constants::MAX_VERTICES || vertex_limit)
						break;
				}
				if (cubes_vertex_count >= Constants::MAX_VERTICES || vertex_limit)
					break;
			}

			// Log efficiency stats
			int total_possible = total_voxels_x * total_voxels_y * total_voxels_z;
			float efficiency = (total_possible > 0) ? (100.0f * skipped_voxels / total_possible) : 0.0f;
			log_message(String("Voxel grid (HEIGHTMAP): {0} vertices, {1}% voxels skipped due to surface band")
								.format(Array::make(cubes_vertex_count, int(efficiency))),
					2);
		} else {
			// Voxels-first mode: process all voxels
			for (int ix = 0; ix < total_voxels_x; ++ix) {
				for (int iy = 0; iy < total_voxels_y; ++iy) {
					for (int iz = 0; iz < total_voxels_z; ++iz) {
						if (cubes_vertex_count >= Constants::MAX_VERTICES || vertex_limit) {
							break;
						}

						// Calculate center position
						Vector3 center;
						center.x = -physical_extent.x * 0.5f + (ix + 0.5f) * eff_voxel_size.x;
						center.y = -physical_extent.y * 0.5f + (iy + 0.5f) * eff_voxel_size.y;
						center.z = -physical_extent.z * 0.5f + (iz + 0.5f) * eff_voxel_size.z;

						// Check center density - only draw for solid voxels
						float center_value = get_terrain_density(center);
						if (center_value >= cutoff) {
							continue; // Skip air voxels
						}

						// Draw cube wireframe
						Vector<Vector3> cube_vertices = create_cube_vertices(center);
						const int edges[12][2] = {
							{ 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 },
							{ 0, 4 }, { 2, 6 }, { 5, 6 }, { 5, 4 },
							{ 5, 1 }, { 6, 7 }, { 4, 7 }, { 3, 7 }
						};
						for (int e = 0; e < 12; ++e) {
							mesh_cubes->surface_set_color(grid_color);
							mesh_cubes->surface_add_vertex(cube_vertices[edges[e][0]]);
							mesh_cubes->surface_add_vertex(cube_vertices[edges[e][1]]);
						}
						cubes_vertex_count += 24;
					}
					if (cubes_vertex_count >= Constants::MAX_VERTICES || vertex_limit)
						break;
				}
				if (cubes_vertex_count >= Constants::MAX_VERTICES || vertex_limit)
					break;
			}

			log_message(String("Voxel grid (VOXELS_FIRST): {0} vertices").format(Array::make(cubes_vertex_count)), 2);
		}

		if (cubes_vertex_count > 0) {
			mesh_cubes->surface_end();

			Ref<StandardMaterial3D> material_cubes;
			material_cubes.instantiate();
			material_cubes->set_flag(godot::BaseMaterial3D::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
			material_cubes->set_shading_mode(BaseMaterial3D::SHADING_MODE_UNSHADED);
			mesh_cubes->surface_set_material(0, material_cubes);
		}
	}

	MeshInstance3D *mi_cubes = memnew(MeshInstance3D);
	mi_cubes->set_name("MeshInstanceVoxelGrid");
	mi_cubes->set_visible(show_voxel_grid);
	mi_cubes->set_mesh(mesh_cubes);
	add_child(mi_cubes);

	// Note: Terrain mesh is now rendered per-chunk via Chunk::apply_mesh_data()
	// Each chunk has its own MeshInstance3D child node

	// # Create chunk grid visualization (red, thicker lines)
	Ref<ImmediateMesh> mesh_chunk_grid;
	mesh_chunk_grid.instantiate();
	mesh_chunk_grid->surface_begin(Mesh::PRIMITIVE_TRIANGLES); // Use triangles for thicker lines

	int chunk_grid_vertex_count = 0;
	float line_thickness = 0.1f; // Thickness of chunk boundary lines
	Color chunk_grid_color(1.0f, 0.0f, 0.0f); // Red color

	// Draw chunk boundaries as thick lines (using thin quads)
	float half_extent_x = physical_extent.x * 0.5f;
	float half_extent_y = physical_extent.y * 0.5f;
	float half_extent_z = physical_extent.z * 0.5f;

	// Helper lambda to draw a thick line as a quad
	auto draw_thick_line = [&](Vector3 start, Vector3 end, Vector3 up) {
		Vector3 dir = (end - start).normalized();
		Vector3 side = dir.cross(up).normalized() * line_thickness * 0.5f;

		Vector3 v0 = start - side;
		Vector3 v1 = start + side;
		Vector3 v2 = end + side;
		Vector3 v3 = end - side;

		mesh_chunk_grid->surface_set_color(chunk_grid_color);
		mesh_chunk_grid->surface_add_vertex(v0);
		mesh_chunk_grid->surface_add_vertex(v1);
		mesh_chunk_grid->surface_add_vertex(v2);

		mesh_chunk_grid->surface_set_color(chunk_grid_color);
		mesh_chunk_grid->surface_add_vertex(v0);
		mesh_chunk_grid->surface_add_vertex(v2);
		mesh_chunk_grid->surface_add_vertex(v3);

		chunk_grid_vertex_count += 6;
	};

	// Draw lines for each chunk boundary
	for (int cx = 0; cx <= world_size.x; ++cx) {
		for (int cy = 0; cy <= world_size.y; ++cy) {
			for (int cz = 0; cz <= world_size.z; ++cz) {
				float x = -half_extent_x + cx * chunk_size;
				float y = -half_extent_y + cy * chunk_size;
				float z = -half_extent_z + cz * chunk_size;

				// Draw edges along X axis
				if (cx < world_size.x) {
					Vector3 start(x, y, z);
					Vector3 end(x + chunk_size, y, z);
					draw_thick_line(start, end, Vector3(0, 1, 0));
				}

				// Draw edges along Y axis
				if (cy < world_size.y) {
					Vector3 start(x, y, z);
					Vector3 end(x, y + chunk_size, z);
					draw_thick_line(start, end, Vector3(1, 0, 0));
				}

				// Draw edges along Z axis
				if (cz < world_size.z) {
					Vector3 start(x, y, z);
					Vector3 end(x, y, z + chunk_size);
					draw_thick_line(start, end, Vector3(0, 1, 0));
				}
			}
		}
	}

	if (chunk_grid_vertex_count > 0) {
		mesh_chunk_grid->surface_end();

		// Create chunk grid material (red, unshaded)
		Ref<StandardMaterial3D> material_chunk_grid;
		material_chunk_grid.instantiate();
		material_chunk_grid->set_albedo(Color(1.0f, 0.0f, 0.0f)); // Red
		material_chunk_grid->set_shading_mode(BaseMaterial3D::SHADING_MODE_UNSHADED);
		material_chunk_grid->set_flag(BaseMaterial3D::FLAG_DISABLE_DEPTH_TEST, false);

		mesh_chunk_grid->surface_set_material(0, material_chunk_grid);

		// Create chunk grid mesh instance
		MeshInstance3D *mi_chunk_grid = memnew(MeshInstance3D);
		mi_chunk_grid->set_name("MeshInstanceChunkGrid");
		mi_chunk_grid->set_visible(show_chunk_grid);
		mi_chunk_grid->set_mesh(mesh_chunk_grid);
		add_child(mi_chunk_grid);
	}

	if (visualize_noise_values) {
		log_message("Creating noise visualization", 2);
		visualize_noise_field();
	}

	// Mark cache as valid so it won't be rebuilt unnecessarily during play
	// Cache will be cleared only when fundamental parameters change (resolution, LOD, mode, world size)
	cache_is_valid = true;

	// Mark generation complete
	generation_in_progress.store(false);

	log_message("VoxelGenerator::generate() completed", 2);
}

// ============================================================================
// Generation Mode: Voxels First (Full 3D Marching Cubes)
// ============================================================================

void VoxelGenerator::generate_voxels_first(Ref<ImmediateMesh> mesh_centers, Ref<ImmediateMesh> mesh_cubes,
		Ref<ImmediateMesh> mesh_triangles, int &centers_vertex_count, int &cubes_vertex_count, int &vertex_count) {
	log_message("Starting VOXELS_FIRST generation mode", 2);

	// Use effective resolution (accounts for LOD)
	int eff_resolution = get_effective_resolution();

	// Compute total marching cubes samples per axis
	int total_voxels_x = std::max(1, world_size.x) * std::max(1, chunk_size) * eff_resolution;
	int total_voxels_y = std::max(1, world_size.y) * std::max(1, chunk_size) * eff_resolution;
	int total_voxels_z = std::max(1, world_size.z) * std::max(1, chunk_size) * eff_resolution;

	// Physical extent stays the same regardless of resolution
	float physical_extent_x = static_cast<float>(std::max(1, world_size.x) * std::max(1, chunk_size));
	float physical_extent_y = static_cast<float>(std::max(1, world_size.y) * std::max(1, chunk_size));
	float physical_extent_z = static_cast<float>(std::max(1, world_size.z) * std::max(1, chunk_size));
	Vector3 physical_extent = Vector3(physical_extent_x, physical_extent_y, physical_extent_z);

	// Voxel size adjusted for effective resolution
	Vector3 eff_voxel_size = Vector3(1.0f, 1.0f, 1.0f) / static_cast<float>(std::max(1, eff_resolution));

	int total_cubes = total_voxels_x * total_voxels_y * total_voxels_z;
	int current_cube = 0;
	int triangle_count = 0;

	log_message(String("Total voxels: {0}x{1}x{2} = {3}").format(Array::make(total_voxels_x, total_voxels_y, total_voxels_z, total_cubes)), 2);

	int non_empty_lookup_count = 0;
	std::vector<int> sample_lookup_indices;
	std::vector<std::vector<float>> sample_cube_values;

	for (int ix = 0; ix < total_voxels_x; ++ix) {
		for (int iy = 0; iy < total_voxels_y; ++iy) {
			for (int iz = 0; iz < total_voxels_z; ++iz) {
				current_cube++;

				// Check vertex limits before adding vertices
				if (vertex_count >= Constants::MAX_VERTICES || vertex_limit) {
					log_message("Vertex limit reached, stopping generation", 1);
					break;
				}

				if (debug_mode && debug_verbosity >= 3) {
					if (current_cube % 1000 == 0 || current_cube == total_cubes) {
						log_message(String("Processing cube {0}/{1} ({2}%)")
											.format(Array::make(current_cube, total_cubes, int(100.0f * current_cube / total_cubes))),
								3);
					}
				}

				// Calculate the center position of the voxel
				Vector3 center;
				center.x = -physical_extent.x * 0.5f + (ix + 0.5f) * eff_voxel_size.x;
				center.y = -physical_extent.y * 0.5f + (iy + 0.5f) * eff_voxel_size.y;
				center.z = -physical_extent.z * 0.5f + (iz + 0.5f) * eff_voxel_size.z;

				float center_value = get_terrain_density(center);

				// Create marching cube vertices
				Vector<Vector3> cube_vertices = create_cube_vertices(center);
				std::vector<float> cube_values = get_cube_values_cached(ix, iy, iz);

				if (center_value < cutoff) {
					add_cubes_vertices(mesh_cubes, cube_vertices);
					cubes_vertex_count += 24;
				}

				int lookup_index = get_lookup_index(cube_values, cutoff);
				const auto &marching_triangles = Constants::get_marching_triangles();

				if (lookup_index >= marching_triangles.size()) {
					continue;
				}

				std::vector<int> triangles(marching_triangles[lookup_index].begin(), marching_triangles[lookup_index].end());

				if (!triangles.empty() && triangles[0] != -1) {
					non_empty_lookup_count++;
					if (sample_lookup_indices.size() < 5) {
						sample_lookup_indices.push_back(lookup_index);
						sample_cube_values.emplace_back(cube_values);
					}
				}

				Color color(
						(center.x + total_voxels_x * 0.5f) / (float)total_voxels_x,
						(center.y + total_voxels_y * 0.5f) / (float)total_voxels_y,
						(center.z + total_voxels_z * 0.5f) / (float)total_voxels_z);

				if (!triangles.empty() && triangles[0] != -1) {
					mesh_centers->surface_set_color(color);
					mesh_centers->surface_add_vertex(center);
					centers_vertex_count++;
				}

				for (size_t index = 0; index < triangles.size(); index += 3) {
					int point_1 = triangles[index];
					if (point_1 == -1)
						continue;
					int point_2 = triangles[index + 1];
					if (point_2 == -1)
						continue;
					int point_3 = triangles[index + 2];
					if (point_3 == -1)
						continue;

					triangle_count++;

					int a0 = Constants::cornerIndexAFromEdge[point_1];
					int b0 = Constants::cornerIndexBFromEdge[point_1];
					int a1 = Constants::cornerIndexAFromEdge[point_2];
					int b1 = Constants::cornerIndexBFromEdge[point_2];
					int a2 = Constants::cornerIndexAFromEdge[point_3];
					int b2 = Constants::cornerIndexBFromEdge[point_3];

					Vector3 vertex1 = interpolate(cube_vertices[a0], cube_values[a0], cube_vertices[b0], cube_values[b0]);
					Vector3 vertex2 = interpolate(cube_vertices[a1], cube_values[a1], cube_vertices[b1], cube_values[b1]);
					Vector3 vertex3 = interpolate(cube_vertices[a2], cube_values[a2], cube_vertices[b2], cube_values[b2]);

					Vector3 vector_a = vertex3 - vertex1;
					Vector3 vector_b = vertex2 - vertex1;
					Vector3 normal = vector_a.cross(vector_b).normalized();

					mesh_triangles->surface_set_color(color);
					mesh_triangles->surface_set_normal(normal);

					if (vertex_count < Constants::MAX_VERTICES && !vertex_limit) {
						mesh_triangles->surface_add_vertex(vertex1);
						mesh_triangles->surface_add_vertex(vertex2);
						mesh_triangles->surface_add_vertex(vertex3);
						vertex_count += 3;
					}
				}
			}
		}
	}

	log_message(String("VOXELS_FIRST completed: {0} triangles, {1} vertices").format(Array::make(triangle_count, vertex_count)), 2);
}

// ============================================================================
// Generation Mode: Heightmap First (Surface Band Optimization)
// ============================================================================

void VoxelGenerator::generate_heightmap_first(Ref<ImmediateMesh> mesh_centers, Ref<ImmediateMesh> mesh_cubes,
		Ref<ImmediateMesh> mesh_triangles, int &centers_vertex_count, int &cubes_vertex_count, int &vertex_count) {
	log_message("Starting HEIGHTMAP_FIRST generation mode", 2);

	// Use effective resolution and surface band (accounts for LOD)
	int eff_resolution = get_effective_resolution();
	float eff_surface_band = get_effective_surface_band();

	// Compute total marching cubes samples per axis
	int total_voxels_x = std::max(1, world_size.x) * std::max(1, chunk_size) * eff_resolution;
	int total_voxels_y = std::max(1, world_size.y) * std::max(1, chunk_size) * eff_resolution;
	int total_voxels_z = std::max(1, world_size.z) * std::max(1, chunk_size) * eff_resolution;

	// Physical extent
	float physical_extent_x = static_cast<float>(std::max(1, world_size.x) * std::max(1, chunk_size));
	float physical_extent_y = static_cast<float>(std::max(1, world_size.y) * std::max(1, chunk_size));
	float physical_extent_z = static_cast<float>(std::max(1, world_size.z) * std::max(1, chunk_size));
	Vector3 physical_extent = Vector3(physical_extent_x, physical_extent_y, physical_extent_z);

	// Voxel size adjusted for effective resolution
	Vector3 eff_voxel_size = Vector3(1.0f, 1.0f, 1.0f) / static_cast<float>(std::max(1, eff_resolution));

	int triangle_count = 0;
	int skipped_voxels = 0;

	log_message(String("Heightmap mode: surface_band={0}, effective_band={1}")
						.format(Array::make(surface_band, eff_surface_band)),
			2);

	// Iterate over X-Z plane first (heightmap driven)
	for (int ix = 0; ix < total_voxels_x; ++ix) {
		for (int iz = 0; iz < total_voxels_z; ++iz) {
			// Get the terrain height at this X-Z position from heightmap cache
			float terrain_height_at_xz = get_height_at(ix, iz);

			// Convert world height to voxel Y coordinate
			// Height is in world units, need to convert to voxel index
			float world_y_center = terrain_height_at_xz;

			// Calculate Y range to process (surface band)
			float y_min_world = world_y_center - eff_surface_band;
			float y_max_world = world_y_center + eff_surface_band;

			// Convert to voxel indices (corner-space, same as density cache)
			int iy_min = std::max(0, static_cast<int>(std::floor((y_min_world + physical_extent.y * 0.5f) / eff_voxel_size.y)));
			int iy_max = std::min(total_voxels_y - 1, static_cast<int>(std::floor((y_max_world + physical_extent.y * 0.5f) / eff_voxel_size.y)));

			// Only process voxels within the surface band
			for (int iy = iy_min; iy <= iy_max; ++iy) {
				// Check vertex limits before adding vertices
				if (vertex_count >= heightmap_vertex_limit || vertex_limit) {
					break;
				}

				// Calculate the center position of the voxel
				Vector3 center;
				center.x = -physical_extent.x * 0.5f + (ix + 0.5f) * eff_voxel_size.x;
				center.y = -physical_extent.y * 0.5f + (iy + 0.5f) * eff_voxel_size.y;
				center.z = -physical_extent.z * 0.5f + (iz + 0.5f) * eff_voxel_size.z;

				float center_value = get_terrain_density(center);

				// Create marching cube vertices
				Vector<Vector3> cube_vertices = create_cube_vertices(center);
				std::vector<float> cube_values = get_cube_values_cached(ix, iy, iz);

				if (center_value < cutoff) {
					add_cubes_vertices(mesh_cubes, cube_vertices);
					cubes_vertex_count += 24;
				}

				int lookup_index = get_lookup_index(cube_values, cutoff);
				const auto &marching_triangles = Constants::get_marching_triangles();

				if (lookup_index >= marching_triangles.size()) {
					continue;
				}

				std::vector<int> triangles(marching_triangles[lookup_index].begin(), marching_triangles[lookup_index].end());

				Color color(
						(center.x + total_voxels_x * 0.5f) / (float)total_voxels_x,
						(center.y + total_voxels_y * 0.5f) / (float)total_voxels_y,
						(center.z + total_voxels_z * 0.5f) / (float)total_voxels_z);

				if (!triangles.empty() && triangles[0] != -1) {
					mesh_centers->surface_set_color(color);
					mesh_centers->surface_add_vertex(center);
					centers_vertex_count++;
				}

				for (size_t index = 0; index < triangles.size(); index += 3) {
					int point_1 = triangles[index];
					if (point_1 == -1)
						continue;
					int point_2 = triangles[index + 1];
					if (point_2 == -1)
						continue;
					int point_3 = triangles[index + 2];
					if (point_3 == -1)
						continue;

					triangle_count++;

					int a0 = Constants::cornerIndexAFromEdge[point_1];
					int b0 = Constants::cornerIndexBFromEdge[point_1];
					int a1 = Constants::cornerIndexAFromEdge[point_2];
					int b1 = Constants::cornerIndexBFromEdge[point_2];
					int a2 = Constants::cornerIndexAFromEdge[point_3];
					int b2 = Constants::cornerIndexBFromEdge[point_3];

					Vector3 vertex1 = interpolate(cube_vertices[a0], cube_values[a0], cube_vertices[b0], cube_values[b0]);
					Vector3 vertex2 = interpolate(cube_vertices[a1], cube_values[a1], cube_vertices[b1], cube_values[b1]);
					Vector3 vertex3 = interpolate(cube_vertices[a2], cube_values[a2], cube_vertices[b2], cube_values[b2]);

					Vector3 vector_a = vertex3 - vertex1;
					Vector3 vector_b = vertex2 - vertex1;
					Vector3 normal = vector_a.cross(vector_b).normalized();

					mesh_triangles->surface_set_color(color);
					mesh_triangles->surface_set_normal(normal);

					if (vertex_count < heightmap_vertex_limit && !vertex_limit) {
						mesh_triangles->surface_add_vertex(vertex1);
						mesh_triangles->surface_add_vertex(vertex2);
						mesh_triangles->surface_add_vertex(vertex3);
						vertex_count += 3;
					}
				}
			}

			// Count skipped voxels (outside surface band)
			skipped_voxels += (total_voxels_y - (iy_max - iy_min + 1));
		}
	}

	int total_possible = total_voxels_x * total_voxels_y * total_voxels_z;
	float efficiency = (total_possible > 0) ? (100.0f * skipped_voxels / total_possible) : 0.0f;
	log_message(String("HEIGHTMAP_FIRST completed: {0} triangles, {1} vertices, {2}% voxels skipped")
						.format(Array::make(triangle_count, vertex_count, int(efficiency))),
			2);
}

Vector<Vector3> VoxelGenerator::create_cube_vertices(const Vector3 &pos) {
	// Use effective_voxel_size so cube dimensions match LOD-adjusted resolution
	// This ensures cubes align properly with the density cache sampling grid
	Vector3 half = effective_voxel_size * 0.5f;
	log_message(String("Creating cube vertices at position: {0} (half={1})").format(Array::make(pos, half)), 3);
	return Vector<Vector3>{
		Vector3(pos.x - half.x, pos.y - half.y, pos.z - half.z),
		Vector3(pos.x + half.x, pos.y - half.y, pos.z - half.z),
		Vector3(pos.x + half.x, pos.y + half.y, pos.z - half.z),
		Vector3(pos.x - half.x, pos.y + half.y, pos.z - half.z),
		Vector3(pos.x - half.x, pos.y - half.y, pos.z + half.z),
		Vector3(pos.x + half.x, pos.y - half.y, pos.z + half.z),
		Vector3(pos.x + half.x, pos.y + half.y, pos.z + half.z),
		Vector3(pos.x - half.x, pos.y + half.y, pos.z + half.z),
	};
}

void VoxelGenerator::add_cubes_vertices(Ref<ImmediateMesh> mesh, const Vector<Vector3> &cube_vertices) {
	const int lines[][2] = {
		{ 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 },
		{ 0, 4 }, { 2, 6 }, { 5, 6 }, { 5, 4 },
		{ 5, 1 }, { 6, 7 }, { 4, 7 }, { 3, 7 }
	};
	for (const auto &line : lines) {
		mesh->surface_add_vertex(cube_vertices[line[0]]);
		mesh->surface_add_vertex(cube_vertices[line[1]]);
	}
	log_message("Added cube edges to mesh", 3);
}

int VoxelGenerator::get_lookup_index(const std::vector<float> &cube_values, float cutoff) {
	int cube_index = 0;
	if (cube_values[0] < cutoff)
		cube_index |= 1;
	if (cube_values[1] < cutoff)
		cube_index |= 2;
	if (cube_values[2] < cutoff)
		cube_index |= 4;
	if (cube_values[3] < cutoff)
		cube_index |= 8;
	if (cube_values[4] < cutoff)
		cube_index |= 16;
	if (cube_values[5] < cutoff)
		cube_index |= 32;
	if (cube_values[6] < cutoff)
		cube_index |= 64;
	if (cube_values[7] < cutoff)
		cube_index |= 128;

	return cube_index;
}

float VoxelGenerator::get_terrain_density(const Vector3 &pos) const {
	// Determine base terrain height from biome generator if available
	float height = sample_base_height(pos.x, pos.z);

	// Add rocky 3D detail using detail noise
	float rocky_detail = 0.0f;
	if (detail_noise.is_valid() && rock_influence > 0.0f) {
		rocky_detail = detail_noise->get_noise_3d(pos.x, pos.y, pos.z) * rock_influence * terrain_amplitude * 0.5f;
	}

	// Base density: negative = solid (below terrain), positive = air (above terrain)
	float base_density = pos.y - height + rocky_detail;

	// Apply terrain + feature edits
	// Convert world position to voxel index for lookup
	int vx = static_cast<int>(std::floor(pos.x));
	int vy = static_cast<int>(std::floor(pos.y));
	int vz = static_cast<int>(std::floor(pos.z));
	uint64_t key = pack_edit_key(vx, vy, vz);
	float edit_delta = 0.0f;

	{
		std::lock_guard<std::mutex> lock(terrain_edits_mutex);
		auto it = terrain_edits.find(key);
		if (it != terrain_edits.end()) {
			edit_delta += it->second;
		}
	}

	{
		std::lock_guard<std::mutex> lock(feature_edits_mutex);
		auto feature_it = feature_density_edits.find(key);
		if (feature_it != feature_density_edits.end()) {
			edit_delta += feature_it->second;
		}
	}

	base_density -= edit_delta;

	return base_density;
}

float VoxelGenerator::sample_raw_base_height(float world_x, float world_z) const {
	float height = terrain_height;
	if (biome_generator.is_valid()) {
		height = biome_generator->get_blended_height_at(world_x, world_z);
	}
	if (terrain_noise.is_valid()) {
		float noise_value = terrain_noise->get_noise_2d(world_x, world_z);
		height += noise_value * terrain_amplitude;
	}

	if (!std::isfinite(height)) {
		height = 0.0f;
	}

	return height;
}

float VoxelGenerator::sample_base_height(float world_x, float world_z) const {
	float height = sample_raw_base_height(world_x, world_z);
	const float world_height_extent = static_cast<float>(std::max(1, world_size.y) * std::max(1, chunk_size));
	if (world_height_extent <= 0.0f) {
		return height;
	}
	const float min_height = -0.5f * world_height_extent;
	const float max_height = 0.5f * world_height_extent;
	return std::clamp(height, min_height, max_height);
}

std::vector<float> VoxelGenerator::get_cube_values(const Vector<Vector3> &cube_vertices) {
	std::vector<float> values(8);
	for (int i = 0; i < 8; ++i) {
		values[i] = get_terrain_density(cube_vertices[i]);
	}
	return values;
}

void VoxelGenerator::add_cube_edges(Ref<ImmediateMesh> mesh, const std::vector<Vector3> &v) {
	mesh->surface_add_vertex(v[0]);
	mesh->surface_add_vertex(v[1]);
	mesh->surface_add_vertex(v[1]);
	mesh->surface_add_vertex(v[2]);
	mesh->surface_add_vertex(v[2]);
	mesh->surface_add_vertex(v[3]);
	mesh->surface_add_vertex(v[3]);
	mesh->surface_add_vertex(v[0]);

	mesh->surface_add_vertex(v[0]);
	mesh->surface_add_vertex(v[4]);
	mesh->surface_add_vertex(v[1]);
	mesh->surface_add_vertex(v[5]);
	mesh->surface_add_vertex(v[2]);
	mesh->surface_add_vertex(v[6]);
	mesh->surface_add_vertex(v[3]);
	mesh->surface_add_vertex(v[7]);

	mesh->surface_add_vertex(v[4]);
	mesh->surface_add_vertex(v[5]);
	mesh->surface_add_vertex(v[5]);
	mesh->surface_add_vertex(v[6]);
	mesh->surface_add_vertex(v[6]);
	mesh->surface_add_vertex(v[7]);
	mesh->surface_add_vertex(v[7]);
	mesh->surface_add_vertex(v[4]);
}

Vector3 VoxelGenerator::interpolate(const Vector3 &vertex1, float value1, const Vector3 &vertex2, float value2) {
	float t = (cutoff - value1) / (value2 - value1);
	return Vector3(
			vertex1.x + t * (vertex2.x - vertex1.x),
			vertex1.y + t * (vertex2.y - vertex1.y),
			vertex1.z + t * (vertex2.z - vertex1.z));
}

// Debug methods implementation
void VoxelGenerator::set_debug_mode(bool p_enabled) {
	debug_mode = p_enabled;
	log_message(String("Debug mode set to: {0}").format(Array::make(debug_mode)), 2);
	if (debug_mode) {
		log_message("Debug mode enabled");
	}
}

bool VoxelGenerator::get_debug_mode() const {
	return debug_mode;
}

void VoxelGenerator::set_visualize_noise_values(bool p_enabled) {
	visualize_noise_values = p_enabled;
	log_message(String("Visualize noise values set to: {0}").format(Array::make(visualize_noise_values)), 2);
	if (visualize_noise_values && is_inside_tree()) {
		visualize_noise_field();
	}
}

bool VoxelGenerator::get_visualize_noise_values() const {
	return visualize_noise_values;
}

void VoxelGenerator::set_debug_verbosity(int p_level) {
	debug_verbosity = CLAMP(p_level, 0, 3);
	log_message(String("Debug verbosity set to: {0}").format(Array::make(debug_verbosity)), 2);
}

int VoxelGenerator::get_debug_verbosity() const {
	return debug_verbosity;
}

void VoxelGenerator::debug_print_state() {
	String debug_info = "=====VoxelGenerator Debug Information:=====\n";
	debug_info += String("====Debug Settings====\n");
	debug_info += String("- Debug Mode: {0}\n").format(Array::make(debug_mode));
	debug_info += String("- Debug Verbosity: {0}\n").format(Array::make(debug_verbosity));
	debug_info += String("====Generator Settings====\n");
	debug_info += String("- Chunk Size: {0}\n").format(Array::make(chunk_size));
	debug_info += String("- Max Chunks Per Frame: {0}\n").format(Array::make(max_chunks_per_frame));
	debug_info += String("- World Size: {0}\n").format(Array::make(world_size));
	debug_info += String("- Base Voxel Size: {0}\n").format(Array::make(base_voxel_size));
	debug_info += String("- Effective Voxel Size: {0}\n").format(Array::make(effective_voxel_size));
	debug_info += String("- Terrain Height: {0}\n").format(Array::make(terrain_height));
	debug_info += String("- Terrain Amplitude: {0}\n").format(Array::make(terrain_amplitude));
	debug_info += String("- Rock Influence: {0}\n").format(Array::make(rock_influence));
	debug_info += String("- Resolution: {0}\n").format(Array::make(resolution));
	debug_info += String("- Generation Mode: {0}\n").format(Array::make(generation_mode));
	debug_info += String("- Surface Band: {0}\n").format(Array::make(surface_band));
	debug_info += String("- Heightmap Vertex Limit: {0}\n").format(Array::make(heightmap_vertex_limit));
	debug_info += String("====LOD Settings====\n");
	debug_info += String("- LOD Level: {0}\n").format(Array::make(lod_level));
	debug_info += String("- LOD Reference Position: {0}\n").format(Array::make(lod_reference_position));
	debug_info += String("- LOD Distance Multiplier: {0}\n").format(Array::make(lod_distance_multiplier));
	debug_info += String("- LOD Distances: {0}\n").format(Array::make(lod_distances));
	debug_info += String("====Other Settings====\n");
	debug_info += String("- Signal Every N Chunks: {0}\n").format(Array::make(signal_every_n_chunks));
	debug_info += String("- Cutoff: {0}\n").format(Array::make(cutoff));
	debug_info += String("- Seed: {0}\n").format(Array::make(seeder));
	debug_info += String("- Show Centers: {0}\n").format(Array::make(show_centers));
	debug_info += String("- Show Voxel Grid: {0}\n").format(Array::make(show_voxel_grid));
	debug_info += String("- Show Chunk Grid: {0}\n").format(Array::make(show_chunk_grid));
	debug_info += String("- Auto Generate: {0}\n").format(Array::make(auto_generate));
	debug_info += String("- Vertex Limit: {0}\n").format(Array::make(vertex_limit));

	UtilityFunctions::print(debug_info);
}

void VoxelGenerator::debug_draw_noise_slice(float y_level) {
	log_message(String("Drawing noise slice at y={0}").format(Array::make(y_level)), 3);

	// Create a new mesh for visualizing the noise slice

	Ref<ImmediateMesh> slice_mesh;
	slice_mesh.instantiate();
	slice_mesh->surface_begin(Mesh::PRIMITIVE_TRIANGLES);

	// Calculate total voxels for bounds (voxel_size is 1x1x1)
	int total_voxels_x = std::max(1, world_size.x) * std::max(1, chunk_size);
	int total_voxels_z = std::max(1, world_size.z) * std::max(1, chunk_size);
	float half_extent_x = total_voxels_x * 0.5f;
	float half_extent_z = total_voxels_z * 0.5f;

	int slice_resolution = resolution * 2; // Higher resolution for better visualization
	float step = 1.0f / slice_resolution;

	// First pass: sample densities to find data range for auto-scaling
	float min_density = std::numeric_limits<float>::infinity();
	float max_density = -std::numeric_limits<float>::infinity();
	for (float x = -half_extent_x; x < half_extent_x; x += step) {
		for (float z = -half_extent_z; z < half_extent_z; z += step) {
			// Use terrain density for visualization
			float density = get_terrain_density(Vector3(x, y_level, z));
			if (std::isfinite(density)) {
				min_density = std::min(min_density, density);
				max_density = std::max(max_density, density);
			}
		}
	}

	// Avoid zero range
	float density_range = max_density - min_density;
	if (density_range <= 1e-6f) {
		// Fallback to using cutoff/terrain_amplitude heuristic if range is too small
		min_density = cutoff - terrain_amplitude;
		max_density = cutoff + terrain_amplitude;
		density_range = max_density - min_density;
	}

	// Second pass: generate colored quads using normalized density (0..1) based on sampled min/max
	for (float x = -half_extent_x; x < half_extent_x; x += step) {
		for (float z = -half_extent_z; z < half_extent_z; z += step) {
			float density = get_terrain_density(Vector3(x, y_level, z));
			if (!std::isfinite(density)) {
				density = min_density;
			}
			float norm = (density - min_density) / density_range;
			norm = CLAMP(norm, 0.0f, 1.0f);

			// Map normalized density to color gradient
			Color color;
			if (norm < 0.5f) {
				// lower half -> blue to cyan
				float t = norm * 2.0f; // 0..1 across lower half
				color = Color(0.0f, t, 1.0f);
			} else {
				// upper half -> yellow to red
				float t = (norm - 0.5f) * 2.0f; // 0..1 across upper half
				color = Color(1.0f, 1.0f - t, 0.0f);
			}

			// Draw a small quad for each sample point
			Vector3 v1 = Vector3(x, y_level, z);
			Vector3 v2 = Vector3(x + step, y_level, z);
			Vector3 v3 = Vector3(x + step, y_level, z + step);
			Vector3 v4 = Vector3(x, y_level, z + step);

			// First triangle
			slice_mesh->surface_set_color(color);
			slice_mesh->surface_add_vertex(v1);
			slice_mesh->surface_add_vertex(v2);
			slice_mesh->surface_add_vertex(v3);

			// Second triangle
			slice_mesh->surface_set_color(color);
			slice_mesh->surface_add_vertex(v1);
			slice_mesh->surface_add_vertex(v3);
			slice_mesh->surface_add_vertex(v4);
		}
	}

	slice_mesh->surface_end();

	// Create material
	Ref<StandardMaterial3D> slice_material;
	slice_material.instantiate();
	slice_material->set_flag(godot::BaseMaterial3D::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
	slice_material->set_shading_mode(BaseMaterial3D::SHADING_MODE_UNSHADED);
	slice_material->set_transparency(BaseMaterial3D::TRANSPARENCY_ALPHA);

	slice_mesh->surface_set_material(0, slice_material);

	// Create mesh instance and add it to the scene
	MeshInstance3D *mi_slice = memnew(MeshInstance3D);
	mi_slice->set_name(String("NoiseSlice_Y{0}").format(Array::make(y_level)));
	mi_slice->set_mesh(slice_mesh);
	add_child(mi_slice);

	log_message("Noise slice visualization created", 2);
}

void VoxelGenerator::log_message(const String &message, int verbosity_level) {
	if (!debug_mode && verbosity_level <= 1) {
		// Always print critical messages (level 1) even if debug mode is off
		UtilityFunctions::print(String("[VoxelGenerator] {0}").format(Array::make(message)));
	} else if (debug_mode && verbosity_level <= debug_verbosity) {
		// Print message if debug mode is on and message verbosity is within our debug level
		UtilityFunctions::print(String("[VoxelGenerator][DEBUG-{0}] {1}").format(Array::make(verbosity_level, message)));
	}
}

void VoxelGenerator::visualize_noise_field() {
	// Create a visualization of the 3D noise field at the zero plane
	debug_draw_noise_slice(0.0f);
}

void VoxelGenerator::create_debug_visualization() {
	if (!debug_mode) {
		log_message("Debug mode is not enabled, skipping visualization.", 1);
		return;
	}

	log_message("Creating debug visualization...", 2);
	visualize_noise_field();
}

void VoxelGenerator::create_chunks() {
	// Clear existing chunks
	for (Chunk *chunk : chunks) {
		if (chunk && is_instance_valid(chunk)) {
			remove_child(chunk);
			chunk->queue_free(); // Use queue_free() instead of memdelete for nodes
		}
	}
	chunks.clear();

	int num_chunks = world_size.x * world_size.y * world_size.z;
	chunk_dirty_flags.resize(num_chunks, false);

	// Create new chunks properly
	for (int x = 0; x < world_size.x; x++) {
		for (int y = 0; y < world_size.y; y++) {
			for (int z = 0; z < world_size.z; z++) {
				Chunk *chunk = memnew(Chunk);

				// Set chunk coordinate and name
				Vector3i coord(x, y, z);
				chunk->set_chunk_coord(coord);
				chunk->set_name(String("Chunk_{0}_{1}_{2}").format(Array::make(x, y, z)));
				chunk->set_chunk_size(chunk_size);

				// Chunk position is (0,0,0) because mesh vertices are in world-space
				// The mesh data already contains world-space coordinates
				chunk->set_position(Vector3(0, 0, 0));
				chunk->set_terrain_material(terrain_material);

				add_child(chunk); // Add to scene tree

				chunks.push_back(chunk);
				fill_chunk_with_voxels(chunk);
			}
		}
	}
}

void VoxelGenerator::fill_chunk_with_voxels(Chunk *chunk) {
	for (int x = 0; x < chunk->get_chunk_size(); ++x) {
		for (int y = 0; y < chunk->get_chunk_size(); ++y) {
			for (int z = 0; z < chunk->get_chunk_size(); ++z) {
				chunk->set_voxel(Vector3i(x, y, z), VoxelType::DIRT);
			}
		}
	}
}

// ============================================================================
// Biome & Feature Helpers
// ============================================================================

Vector3 VoxelGenerator::get_chunk_world_origin(const Vector3i &chunk_coord) const {
	Vector3 world_extent(
			static_cast<float>(std::max(1, world_size.x) * std::max(1, chunk_size)),
			static_cast<float>(std::max(1, world_size.y) * std::max(1, chunk_size)),
			static_cast<float>(std::max(1, world_size.z) * std::max(1, chunk_size)));
	return Vector3(-world_extent.x * 0.5f + static_cast<float>(chunk_coord.x * chunk_size),
			-world_extent.y * 0.5f + static_cast<float>(chunk_coord.y * chunk_size),
			-world_extent.z * 0.5f + static_cast<float>(chunk_coord.z * chunk_size));
}

void VoxelGenerator::ensure_vertical_extent_for_biomes() {
	// DISABLED: Auto-expansion causes terrain to float above chunk grid
	// because biome heights (e.g., 45-55) aren't centered around Y=0.
	// User should manually set world_size.y to desired value.
	// sample_base_height() already clamps heights to fit within world bounds.

	if (!biome_generator.is_valid()) {
		return;
	}

	// Just finalize immediately without expansion
	world_size_finalized = true;
	world_size_locked = world_size;

	log_message(String("World size locked at ({0}, {1}, {2}) - biome heights will be clamped to fit")
						.format(Array::make(world_size.x, world_size.y, world_size.z)),
			1);
}

TypedArray<int> VoxelGenerator::get_chunk_surface_heights(const Vector3i &chunk_coord) const {
	TypedArray<int> heights;
	if (chunk_size <= 0) {
		return heights;
	}

	heights.resize(chunk_size * chunk_size);
	Vector3 origin = get_chunk_world_origin(chunk_coord);
	const float world_height_extent = static_cast<float>(std::max(1, world_size.y) * std::max(1, chunk_size));
	const float min_height = -0.5f * world_height_extent;
	const float max_height = 0.5f * world_height_extent;

	for (int lx = 0; lx < chunk_size; ++lx) {
		for (int lz = 0; lz < chunk_size; ++lz) {
			float world_x = origin.x + static_cast<float>(lx) + 0.5f;
			float world_z = origin.z + static_cast<float>(lz) + 0.5f;
			float height = sample_base_height(world_x, world_z);
			if (!std::isfinite(height)) {
				height = 0.0f;
			} else {
				height = std::clamp(height, min_height, max_height);
			}
			int column_index = lx * chunk_size + lz;
			heights[column_index] = static_cast<int>(std::floor(height));
		}
	}

	return heights;
}

TypedArray<int> VoxelGenerator::get_chunk_biome_indices(const Vector3i &chunk_coord) const {
	TypedArray<int> indices;
	if (chunk_size <= 0) {
		return indices;
	}

	indices.resize(chunk_size * chunk_size);
	Vector3 origin = get_chunk_world_origin(chunk_coord);
	bool has_biomes = biome_generator.is_valid();

	for (int lx = 0; lx < chunk_size; ++lx) {
		for (int lz = 0; lz < chunk_size; ++lz) {
			float world_x = origin.x + static_cast<float>(lx) + 0.5f;
			float world_z = origin.z + static_cast<float>(lz) + 0.5f;
			int biome_index = has_biomes ? biome_generator->get_biome_index_at(world_x, world_z) : 0;
			int column_index = lx * chunk_size + lz;
			indices[column_index] = biome_index < 0 ? 0 : biome_index;
		}
	}

	return indices;
}

void VoxelGenerator::apply_feature_density_sphere(const Vector3 &center, float radius, float delta) {
	if (radius <= 0.0f || delta == 0.0f) {
		return;
	}

	int min_x = static_cast<int>(std::floor(center.x - radius));
	int min_y = static_cast<int>(std::floor(center.y - radius));
	int min_z = static_cast<int>(std::floor(center.z - radius));
	int max_x = static_cast<int>(std::ceil(center.x + radius));
	int max_y = static_cast<int>(std::ceil(center.y + radius));
	int max_z = static_cast<int>(std::ceil(center.z + radius));
	float radius_sq = radius * radius;

	std::lock_guard<std::mutex> lock(feature_edits_mutex);
	for (int vz = min_z; vz <= max_z; ++vz) {
		for (int vy = min_y; vy <= max_y; ++vy) {
			for (int vx = min_x; vx <= max_x; ++vx) {
				float dx = (static_cast<float>(vx) + 0.5f) - center.x;
				float dy = (static_cast<float>(vy) + 0.5f) - center.y;
				float dz = (static_cast<float>(vz) + 0.5f) - center.z;
				float dist_sq = dx * dx + dy * dy + dz * dz;
				if (dist_sq > radius_sq) {
					continue;
				}
				float dist = Math::sqrt(dist_sq);
				float t = radius == 0.0f ? 0.0f : dist / radius;
				float falloff = 1.0f - (t * t * (3.0f - 2.0f * t));
				float edit_delta = delta * falloff;
				uint64_t key = pack_edit_key(vx, vy, vz);
				auto found = feature_density_edits.find(key);
				if (found != feature_density_edits.end()) {
					found->second += edit_delta;
					if (std::abs(found->second) < 0.001f) {
						feature_density_edits.erase(found);
					}
				} else {
					feature_density_edits[key] = edit_delta;
				}
			}
		}
	}
}

void VoxelGenerator::bake_feature_descriptor(const Dictionary &feature) {
	if (!feature.has("position") || !feature.has("feature_type")) {
		return;
	}

	Vector3i position = feature["position"];
	int feature_type_int = static_cast<int>(feature["feature_type"]);
	int feature_size = feature.has("size") ? static_cast<int>(feature["size"]) : 1;
	Vector3 center(static_cast<float>(position.x), static_cast<float>(position.y), static_cast<float>(position.z));
	FeatureType feature_type = static_cast<FeatureType>(feature_type_int);

	switch (feature_type) {
		case FeatureType::TREE: {
			int trunk_height = CLAMP(feature_size, 2, 8);
			for (int i = 0; i < trunk_height; ++i) {
				apply_feature_density_sphere(center + Vector3(0.0f, static_cast<float>(i), 0.0f), 0.6f, 3.0f);
			}
			float canopy_radius = 1.5f + static_cast<float>(feature_size) * 0.3f;
			apply_feature_density_sphere(center + Vector3(0.0f, static_cast<float>(trunk_height), 0.0f), canopy_radius, 2.0f);
			break;
		}
		case FeatureType::ROCK: {
			float radius = 0.8f + static_cast<float>(feature_size) * 0.25f;
			apply_feature_density_sphere(center, radius, 2.5f);
			break;
		}
		case FeatureType::FOLIAGE: {
			float radius = 0.5f + static_cast<float>(feature_size) * 0.1f;
			apply_feature_density_sphere(center, radius, 1.5f);
			break;
		}
		case FeatureType::STRUCTURE: {
			float radius = 2.0f + static_cast<float>(feature_size);
			apply_feature_density_sphere(center, radius, 4.0f);
			break;
		}
		case FeatureType::ORE_VEIN:
		default: {
			if (feature_generator.is_valid()) {
				Array cluster = feature_generator->generate_ore_cluster(position.x, position.y, position.z, Math::max(1, feature_size));
				for (int i = 0; i < cluster.size(); ++i) {
					Vector3i ore_pos = cluster[i];
					apply_feature_density_sphere(
							Vector3(static_cast<float>(ore_pos.x), static_cast<float>(ore_pos.y), static_cast<float>(ore_pos.z)),
							0.6f + static_cast<float>(feature_size) * 0.05f,
							2.0f);
				}
			} else {
				apply_feature_density_sphere(center, 1.0f, 2.0f);
			}
			break;
		}
	}
}

int VoxelGenerator::generate_chunk_features(const Vector3i &chunk_coord) {
	if (!feature_generator.is_valid() || chunk_size <= 0) {
		return 0;
	}

	TypedArray<int> surface_heights = get_chunk_surface_heights(chunk_coord);
	TypedArray<int> biome_indices = get_chunk_biome_indices(chunk_coord);
	Vector3 origin = get_chunk_world_origin(chunk_coord);
	int start_x = static_cast<int>(std::floor(origin.x));
	int start_y = static_cast<int>(std::floor(origin.y));
	int start_z = static_cast<int>(std::floor(origin.z));

	Array features = feature_generator->get_features_in_region(
			start_x,
			start_y,
			start_z,
			chunk_size,
			chunk_size,
			chunk_size,
			surface_heights,
			biome_indices);

	for (int i = 0; i < features.size(); ++i) {
		Dictionary feature = features[i];
		bake_feature_descriptor(feature);
	}

	return features.size();
}

void VoxelGenerator::prepare_procedural_features() {
	{
		std::lock_guard<std::mutex> lock(feature_edits_mutex);
		feature_density_edits.clear();
	}

	if (!feature_generator.is_valid()) {
		log_message("Feature generator not assigned, skipping procedural feature pass", 3);
		return;
	}

	if (!biome_generator.is_valid()) {
		log_message("Feature generator requires a biome generator; skipping procedural features", 1);
		return;
	}

	if (chunks.empty()) {
		create_chunks();
	}

	int total_features = 0;
	for (Chunk *chunk : chunks) {
		if (!chunk || !is_instance_valid(chunk)) {
			continue;
		}
		total_features += apply_features_to_chunk(chunk);
	}

	log_message(String("Baked {0} procedural features into terrain").format(Array::make(total_features)), 2);
}

Color VoxelGenerator::get_biome_debug_color(int biome_index) const {
	static const Color palette[] = {
		Color(0.42f, 0.76f, 0.46f), // plains
		Color(0.85f, 0.74f, 0.45f), // desert
		Color(0.55f, 0.66f, 0.82f), // snow/mountain
		Color(0.54f, 0.78f, 0.93f), // ocean
		Color(0.78f, 0.56f, 0.42f), // mesa
		Color(0.64f, 0.86f, 0.52f) // forest
	};
	constexpr int palette_size = static_cast<int>(sizeof(palette) / sizeof(Color));
	if (biome_index < 0) {
		return Color(0.6f, 0.6f, 0.6f);
	}
	return palette[biome_index % palette_size];
}

int VoxelGenerator::apply_features_to_chunk(Chunk *chunk) {
	if (!chunk) {
		return 0;
	}
	return generate_chunk_features(chunk->get_chunk_coord());
}

// ============================================================================
// Density Cache Implementation
// ============================================================================

void VoxelGenerator::build_density_cache() {
	// Calculate cache dimensions (all corner positions for marching cubes)
	// Use effective resolution (accounts for LOD) to match generation loop spacing
	int eff_resolution = get_effective_resolution();

	// Store the resolution and voxel size used for this cache
	// This is critical for LOD-independent sampling via trilinear interpolation
	cache_resolution = eff_resolution;
	cache_voxel_size = effective_voxel_size;

	int voxel_count_x = world_size.x * chunk_size * eff_resolution;
	int voxel_count_y = world_size.y * chunk_size * eff_resolution;
	int voxel_count_z = world_size.z * chunk_size * eff_resolution;

	cache_size_x = voxel_count_x + 1;
	cache_size_y = voxel_count_y + 1;
	cache_size_z = voxel_count_z + 1;

	size_t total_size = static_cast<size_t>(cache_size_x) * cache_size_y * cache_size_z;

	log_message(String("Building density cache: {0}x{1}x{2} = {3} entries").format(Array::make(cache_size_x, cache_size_y, cache_size_z, static_cast<int64_t>(total_size))), 2);

	{
		std::unique_lock<std::shared_mutex> lock(density_cache_mutex);
		density_cache.resize(total_size);

		// Fill the cache with density values
		// Physical extent stays constant regardless of resolution
		float physical_extent_x = static_cast<float>(world_size.x * chunk_size);
		float physical_extent_y = static_cast<float>(world_size.y * chunk_size);
		float physical_extent_z = static_cast<float>(world_size.z * chunk_size);
		Vector3 physical_extent(physical_extent_x, physical_extent_y, physical_extent_z);

		for (int iz = 0; iz < cache_size_z; ++iz) {
			for (int iy = 0; iy < cache_size_y; ++iy) {
				for (int ix = 0; ix < cache_size_x; ++ix) {
					// Convert index to world position (corner position, not center)
					// Use effective_voxel_size to match the generation loop spacing
					Vector3 pos;
					pos.x = -physical_extent.x * 0.5f + ix * effective_voxel_size.x;
					pos.y = -physical_extent.y * 0.5f + iy * effective_voxel_size.y;
					pos.z = -physical_extent.z * 0.5f + iz * effective_voxel_size.z;

					int index = density_cache_index(ix, iy, iz);
					density_cache[index] = get_terrain_density(pos);
				}
			}
		}
	}

	log_message("Density cache built successfully", 2);
}

void VoxelGenerator::clear_density_cache() {
	std::unique_lock<std::shared_mutex> lock(density_cache_mutex);
	density_cache.clear();
	density_cache.shrink_to_fit();
	cache_size_x = 0;
	cache_size_y = 0;
	cache_size_z = 0;
	log_message("Density cache cleared", 2);
}

float VoxelGenerator::get_cached_density(int ix, int iy, int iz) const {
	std::shared_lock<std::shared_mutex> lock(density_cache_mutex);

	// Helper lambda to convert indices to world position
	// Physical extent is constant, effective_voxel_size accounts for LOD-adjusted resolution
	auto index_to_pos = [this](int x, int y, int z) -> Vector3 {
		float physical_extent_x = static_cast<float>(world_size.x * chunk_size);
		float physical_extent_y = static_cast<float>(world_size.y * chunk_size);
		float physical_extent_z = static_cast<float>(world_size.z * chunk_size);
		return Vector3(
				-physical_extent_x * 0.5f + x * effective_voxel_size.x,
				-physical_extent_y * 0.5f + y * effective_voxel_size.y,
				-physical_extent_z * 0.5f + z * effective_voxel_size.z);
	};

	if (density_cache.empty()) {
		// Fallback to direct calculation if cache not built
		lock.unlock();
		return get_terrain_density(index_to_pos(ix, iy, iz));
	}

	// Bounds check
	if (ix < 0 || ix >= cache_size_x ||
			iy < 0 || iy >= cache_size_y ||
			iz < 0 || iz >= cache_size_z) {
		lock.unlock();
		return get_terrain_density(index_to_pos(ix, iy, iz));
	}

	// Get base density from cache (procedural terrain without edits)
	float base_density = density_cache[density_cache_index(ix, iy, iz)];

	// Release cache lock before acquiring edit locks to avoid deadlock
	lock.unlock();

	// Early out if no terrain edits exist (common case - no overhead)
	bool has_terrain_edits = false;
	bool has_feature_edits = false;
	{
		std::lock_guard<std::mutex> terrain_lock(terrain_edits_mutex);
		has_terrain_edits = !terrain_edits.empty();
	}
	{
		std::lock_guard<std::mutex> feature_lock(feature_edits_mutex);
		has_feature_edits = !feature_density_edits.empty();
	}

	if (!has_terrain_edits && !has_feature_edits) {
		return base_density;
	}

	// Convert cache index to world position for edit lookup
	Vector3 pos = index_to_pos(ix, iy, iz);
	int vx = static_cast<int>(std::floor(pos.x));
	int vy = static_cast<int>(std::floor(pos.y));
	int vz = static_cast<int>(std::floor(pos.z));
	uint64_t key = pack_edit_key(vx, vy, vz);

	// Apply terrain edits on top of base density
	float edit_delta = 0.0f;

	if (has_terrain_edits) {
		std::lock_guard<std::mutex> terrain_lock(terrain_edits_mutex);
		auto it = terrain_edits.find(key);
		if (it != terrain_edits.end()) {
			edit_delta += it->second;
		}
	}

	if (has_feature_edits) {
		std::lock_guard<std::mutex> feature_lock(feature_edits_mutex);
		auto it = feature_density_edits.find(key);
		if (it != feature_density_edits.end()) {
			edit_delta += it->second;
		}
	}

	// Apply edit delta (same formula as get_terrain_density)
	return base_density - edit_delta;
}

std::vector<float> VoxelGenerator::get_cube_values_cached(int ix, int iy, int iz) const {
	std::vector<float> values(8);

	// Corner ordering must match create_cube_vertices():
	// 0: (-half, -half, -half) -> (ix,   iy,   iz)
	// 1: (+half, -half, -half) -> (ix+1, iy,   iz)
	// 2: (+half, +half, -half) -> (ix+1, iy+1, iz)
	// 3: (-half, +half, -half) -> (ix,   iy+1, iz)
	// 4: (-half, -half, +half) -> (ix,   iy,   iz+1)
	// 5: (+half, -half, +half) -> (ix+1, iy,   iz+1)
	// 6: (+half, +half, +half) -> (ix+1, iy+1, iz+1)
	// 7: (-half, +half, +half) -> (ix,   iy+1, iz+1)
	values[0] = get_cached_density(ix, iy, iz);
	values[1] = get_cached_density(ix + 1, iy, iz);
	values[2] = get_cached_density(ix + 1, iy + 1, iz);
	values[3] = get_cached_density(ix, iy + 1, iz);
	values[4] = get_cached_density(ix, iy, iz + 1);
	values[5] = get_cached_density(ix + 1, iy, iz + 1);
	values[6] = get_cached_density(ix + 1, iy + 1, iz + 1);
	values[7] = get_cached_density(ix, iy + 1, iz + 1);

	return values;
}

float VoxelGenerator::get_density_at_world_position(const Vector3 &world_pos) const {
	// Convert world position to cache index using the cache's voxel size
	// This allows sampling at any resolution while using cache built at different resolution
	float physical_extent_x = static_cast<float>(world_size.x * chunk_size);
	float physical_extent_y = static_cast<float>(world_size.y * chunk_size);
	float physical_extent_z = static_cast<float>(world_size.z * chunk_size);

	// Convert world pos to floating-point cache indices
	float fx = (world_pos.x + physical_extent_x * 0.5f) / cache_voxel_size.x;
	float fy = (world_pos.y + physical_extent_y * 0.5f) / cache_voxel_size.y;
	float fz = (world_pos.z + physical_extent_z * 0.5f) / cache_voxel_size.z;

	// Get integer indices for the 8 surrounding cache cells
	int ix0 = static_cast<int>(std::floor(fx));
	int iy0 = static_cast<int>(std::floor(fy));
	int iz0 = static_cast<int>(std::floor(fz));
	int ix1 = ix0 + 1;
	int iy1 = iy0 + 1;
	int iz1 = iz0 + 1;

	// Fractional parts for interpolation
	float tx = fx - static_cast<float>(ix0);
	float ty = fy - static_cast<float>(iy0);
	float tz = fz - static_cast<float>(iz0);

	// Sample 8 corners from the cache
	float c000 = get_cached_density(ix0, iy0, iz0);
	float c100 = get_cached_density(ix1, iy0, iz0);
	float c010 = get_cached_density(ix0, iy1, iz0);
	float c110 = get_cached_density(ix1, iy1, iz0);
	float c001 = get_cached_density(ix0, iy0, iz1);
	float c101 = get_cached_density(ix1, iy0, iz1);
	float c011 = get_cached_density(ix0, iy1, iz1);
	float c111 = get_cached_density(ix1, iy1, iz1);

	// Trilinear interpolation
	float c00 = c000 * (1.0f - tx) + c100 * tx;
	float c10 = c010 * (1.0f - tx) + c110 * tx;
	float c01 = c001 * (1.0f - tx) + c101 * tx;
	float c11 = c011 * (1.0f - tx) + c111 * tx;

	float c0 = c00 * (1.0f - ty) + c10 * ty;
	float c1 = c01 * (1.0f - ty) + c11 * ty;

	return c0 * (1.0f - tz) + c1 * tz;
}

std::vector<float> VoxelGenerator::get_cube_values_at_world_position(const Vector3 &center, const Vector3 &half_size) const {
	std::vector<float> values(8);

	// Sample density at 8 corners of the cube in world space
	// Corner ordering must match create_cube_vertices():
	// 0: (-half, -half, -half)
	// 1: (+half, -half, -half)
	// 2: (+half, +half, -half)
	// 3: (-half, +half, -half)
	// 4: (-half, -half, +half)
	// 5: (+half, -half, +half)
	// 6: (+half, +half, +half)
	// 7: (-half, +half, +half)
	values[0] = get_density_at_world_position(Vector3(center.x - half_size.x, center.y - half_size.y, center.z - half_size.z));
	values[1] = get_density_at_world_position(Vector3(center.x + half_size.x, center.y - half_size.y, center.z - half_size.z));
	values[2] = get_density_at_world_position(Vector3(center.x + half_size.x, center.y + half_size.y, center.z - half_size.z));
	values[3] = get_density_at_world_position(Vector3(center.x - half_size.x, center.y + half_size.y, center.z - half_size.z));
	values[4] = get_density_at_world_position(Vector3(center.x - half_size.x, center.y - half_size.y, center.z + half_size.z));
	values[5] = get_density_at_world_position(Vector3(center.x + half_size.x, center.y - half_size.y, center.z + half_size.z));
	values[6] = get_density_at_world_position(Vector3(center.x + half_size.x, center.y + half_size.y, center.z + half_size.z));
	values[7] = get_density_at_world_position(Vector3(center.x - half_size.x, center.y + half_size.y, center.z + half_size.z));

	return values;
}

// ============================================================================
// Dirty Flag System Implementation
// ============================================================================

int VoxelGenerator::chunk_coord_to_index(const Vector3i &chunk_coord) const {
	// Flatten 3D chunk coordinate to 1D index
	// Order: X + Y * sizeX + Z * sizeX * sizeY
	if (chunk_coord.x < 0 || chunk_coord.x >= world_size.x ||
			chunk_coord.y < 0 || chunk_coord.y >= world_size.y ||
			chunk_coord.z < 0 || chunk_coord.z >= world_size.z) {
		return -1; // Invalid coordinate
	}
	return chunk_coord.x +
			chunk_coord.y * world_size.x +
			chunk_coord.z * world_size.x * world_size.y;
}

Vector3i VoxelGenerator::index_to_chunk_coord(int index) const {
	int total = world_size.x * world_size.y * world_size.z;
	if (index < 0 || index >= total) {
		return Vector3i(-1, -1, -1);
	}

	int z = index / (world_size.x * world_size.y);
	int remainder = index % (world_size.x * world_size.y);
	int y = remainder / world_size.x;
	int x = remainder % world_size.x;

	return Vector3i(x, y, z);
}

void VoxelGenerator::mark_chunk_dirty(const Vector3i &chunk_coord) {
	std::lock_guard<std::mutex> lock(chunks_mutex);
	int idx = chunk_coord_to_index(chunk_coord);
	if (idx >= 0 && idx < static_cast<int>(chunk_dirty_flags.size())) {
		chunk_dirty_flags[idx] = true;
		log_message(String("Chunk ({0},{1},{2}) marked dirty").format(Array::make(chunk_coord.x, chunk_coord.y, chunk_coord.z)), 3);
	}
}

void VoxelGenerator::mark_all_chunks_dirty() {
	std::lock_guard<std::mutex> lock(chunks_mutex);
	for (auto &flag : chunk_dirty_flags) {
		flag = true;
	}
	log_message("All chunks marked dirty", 2);
}

bool VoxelGenerator::is_chunk_dirty(const Vector3i &chunk_coord) const {
	std::lock_guard<std::mutex> lock(chunks_mutex);
	int idx = chunk_coord_to_index(chunk_coord);
	if (idx >= 0 && idx < static_cast<int>(chunk_dirty_flags.size())) {
		return chunk_dirty_flags[idx];
	}
	return false;
}

void VoxelGenerator::regenerate_dirty_chunks() {
	// Note: We allow terraforming to proceed even during async generation
	// The regeneration will use the existing cached density and apply terrain edits
	// Do NOT set generation_in_progress = true here, as that's for full world generation

	log_message("Regenerating dirty chunks...", 2);

	// Only rebuild density cache if it's invalid or doesn't match current parameters
	if (!cache_is_valid || cache_size_x == 0 || cache_size_y == 0 || cache_size_z == 0 ||
			cache_resolution != get_effective_resolution() || cache_voxel_size != effective_voxel_size) {
		log_message("Cache invalid or params changed - rebuilding density cache", 3);
		build_density_cache();
		cache_is_valid = true;
	} else {
		log_message("Reusing existing density cache", 3);
	} // Synchronously regenerate dirty chunks using per-chunk mesh generation
	int regenerated_count = 0;

	for (int idx = 0; idx < static_cast<int>(chunk_dirty_flags.size()); ++idx) {
		bool is_dirty;
		{
			std::lock_guard<std::mutex> lock(chunks_mutex);
			is_dirty = chunk_dirty_flags[idx];
		}
		if (is_dirty) {
			Vector3i chunk_coord = index_to_chunk_coord(idx);

			// Clear the existing mesh for this chunk
			if (idx < static_cast<int>(chunks.size()) && chunks[idx]) {
				chunks[idx]->clear_mesh();
			}

			// Regenerate this chunk's mesh using the sync method
			generate_chunk_mesh_sync(idx);

			regenerated_count++;

			log_message(String("Regenerated chunk ({0},{1},{2})").format(Array::make(chunk_coord.x, chunk_coord.y, chunk_coord.z)), 3);
		}
	}

	log_message(String("Regenerated {0} dirty chunks").format(Array::make(regenerated_count)), 2);
}

void VoxelGenerator::invalidate_density_region(const Vector3i &min_voxel, const Vector3i &max_voxel) {
	log_message(String("Invalidating density region from ({0},{1},{2}) to ({3},{4},{5})").format(Array::make(min_voxel.x, min_voxel.y, min_voxel.z, max_voxel.x, max_voxel.y, max_voxel.z)), 2);

	if (chunk_size <= 0) {
		log_message("Cannot invalidate density region: chunk_size <= 0", 0);
		return;
	}

	Vector3 world_extent(
			static_cast<float>(std::max(1, world_size.x) * chunk_size),
			static_cast<float>(std::max(1, world_size.y) * chunk_size),
			static_cast<float>(std::max(1, world_size.z) * chunk_size));
	Vector3 half_extent = world_extent * 0.5f;

	auto voxel_to_chunk = [&](int voxel_coord, float half_axis_extent, int axis_chunks) {
		if (axis_chunks <= 0) {
			return 0;
		}
		float shifted = static_cast<float>(voxel_coord) + half_axis_extent;
		int chunk = static_cast<int>(std::floor(shifted / static_cast<float>(chunk_size)));
		if (chunk < 0) {
			chunk = 0;
		} else if (chunk >= axis_chunks) {
			chunk = axis_chunks - 1;
		}
		return chunk;
	};

	// Calculate which chunks are affected by this region using world-centered coordinates
	Vector3i min_chunk(
			voxel_to_chunk(min_voxel.x, half_extent.x, world_size.x),
			voxel_to_chunk(min_voxel.y, half_extent.y, world_size.y),
			voxel_to_chunk(min_voxel.z, half_extent.z, world_size.z));
	Vector3i max_chunk(
			voxel_to_chunk(max_voxel.x, half_extent.x, world_size.x),
			voxel_to_chunk(max_voxel.y, half_extent.y, world_size.y),
			voxel_to_chunk(max_voxel.z, half_extent.z, world_size.z));

	// Ensure ordering after clamping
	min_chunk.x = std::min(min_chunk.x, max_chunk.x);
	min_chunk.y = std::min(min_chunk.y, max_chunk.y);
	min_chunk.z = std::min(min_chunk.z, max_chunk.z);
	max_chunk.x = std::max(min_chunk.x, max_chunk.x);
	max_chunk.y = std::max(min_chunk.y, max_chunk.y);
	max_chunk.z = std::max(min_chunk.z, max_chunk.z);

	// Mark all affected chunks as dirty
	for (int cz = min_chunk.z; cz <= max_chunk.z; ++cz) {
		for (int cy = min_chunk.y; cy <= max_chunk.y; ++cy) {
			for (int cx = min_chunk.x; cx <= max_chunk.x; ++cx) {
				mark_chunk_dirty(Vector3i(cx, cy, cz));
			}
		}
	}

	// Note: We do NOT invalidate the density cache here because:
	// 1. The cache represents the procedural base terrain (which hasn't changed)
	// 2. Terrain edits are applied via the terrain_edits map in get_terrain_density()
	// 3. Keeping the cache valid avoids expensive full rebuilds during terraforming
	log_message("Density region marked for regeneration; affected chunks marked dirty", 3);
}

// ==================== Terraforming Implementation ====================

void VoxelGenerator::modify_terrain(const Vector3 &center, float radius, float delta) {
	if (radius <= 0.0f || delta == 0.0f) {
		return;
	}

	log_message(String("Modifying terrain at ({0},{1},{2}) radius={3} delta={4}").format(Array::make(center.x, center.y, center.z, radius, delta)), 2);

	// Calculate bounding box in voxel coordinates
	int min_x = static_cast<int>(std::floor(center.x - radius));
	int min_y = static_cast<int>(std::floor(center.y - radius));
	int min_z = static_cast<int>(std::floor(center.z - radius));
	int max_x = static_cast<int>(std::ceil(center.x + radius));
	int max_y = static_cast<int>(std::ceil(center.y + radius));
	int max_z = static_cast<int>(std::ceil(center.z + radius));

	float radius_sq = radius * radius;

	// Apply edits within the spherical region
	{
		std::lock_guard<std::mutex> lock(terrain_edits_mutex);
		for (int vz = min_z; vz <= max_z; ++vz) {
			for (int vy = min_y; vy <= max_y; ++vy) {
				for (int vx = min_x; vx <= max_x; ++vx) {
					// Calculate distance from center (use voxel center)
					float dx = (vx + 0.5f) - center.x;
					float dy = (vy + 0.5f) - center.y;
					float dz = (vz + 0.5f) - center.z;
					float dist_sq = dx * dx + dy * dy + dz * dz;

					if (dist_sq <= radius_sq) {
						// Calculate smooth falloff using smoothstep
						// 1.0 at center, 0.0 at edge
						float dist = std::sqrt(dist_sq);
						float t = dist / radius; // 0 at center, 1 at edge
						// Smoothstep: 3t^2 - 2t^3 (inverted for falloff)
						float falloff = 1.0f - (t * t * (3.0f - 2.0f * t));

						float edit_delta = delta * falloff;

						uint64_t key = pack_edit_key(vx, vy, vz);
						auto it = terrain_edits.find(key);
						if (it != terrain_edits.end()) {
							// Accumulate with existing edit
							it->second += edit_delta;
							// Remove entry if delta is negligible
							if (std::abs(it->second) < 0.001f) {
								terrain_edits.erase(it);
							}
						} else {
							// Add new edit
							terrain_edits[key] = edit_delta;
						}
					}
				}
			}
		}
	}

	// Mark affected chunks as dirty and invalidate density cache
	invalidate_density_region(Vector3i(min_x, min_y, min_z), Vector3i(max_x, max_y, max_z));

	log_message(String("Terrain edits count: {0}").format(Array::make(static_cast<int>(terrain_edits.size()))), 2);

	// Immediately regenerate affected chunks to show terrain edits
	regenerate_dirty_chunks();
}

void VoxelGenerator::dig_sphere(const Vector3 &center, float radius, float strength) {
	// Digging removes terrain: positive delta decreases density (makes air)
	// Since density -= edit_delta in get_terrain_density, negative delta increases final density
	// So we use negative strength to increase density (remove solid)
	modify_terrain(center, radius, -strength);
}

void VoxelGenerator::build_sphere(const Vector3 &center, float radius, float strength) {
	// Building adds terrain: negative delta increases density (makes solid)
	// Since density -= edit_delta in get_terrain_density, positive delta decreases final density
	// So we use positive strength to decrease density (add solid)
	modify_terrain(center, radius, strength);
}

void VoxelGenerator::clear_terrain_edits() {
	{
		std::lock_guard<std::mutex> lock(terrain_edits_mutex);
		terrain_edits.clear();
	}
	log_message("Cleared all terrain edits", 1);
	// Mark all chunks dirty to regenerate original terrain
	mark_all_chunks_dirty();
	clear_density_cache();
}

int VoxelGenerator::get_terrain_edit_count() const {
	std::lock_guard<std::mutex> lock(terrain_edits_mutex);
	return static_cast<int>(terrain_edits.size());
}

PackedFloat32Array VoxelGenerator::get_terrain_edits_data() const {
	std::lock_guard<std::mutex> lock(terrain_edits_mutex);
	// Format: [count, x1, y1, z1, delta1, x2, y2, z2, delta2, ...]
	PackedFloat32Array data;
	data.resize(1 + terrain_edits.size() * 4);

	int idx = 0;
	data[idx++] = static_cast<float>(terrain_edits.size());

	for (const auto &pair : terrain_edits) {
		uint64_t key = pair.first;
		// Unpack key back to coordinates
		int x = static_cast<int>((key >> 40) & 0xFFFFF) - 524288;
		int y = static_cast<int>((key >> 20) & 0xFFFFF) - 524288;
		int z = static_cast<int>(key & 0xFFFFF) - 524288;

		data[idx++] = static_cast<float>(x);
		data[idx++] = static_cast<float>(y);
		data[idx++] = static_cast<float>(z);
		data[idx++] = pair.second;
	}

	return data;
}

void VoxelGenerator::set_terrain_edits_data(const PackedFloat32Array &data) {
	if (data.size() < 1) {
		return;
	}

	int count = static_cast<int>(data[0]);
	if (data.size() < 1 + count * 4) {
		log_message("Invalid terrain edits data size", 1);
		return;
	}

	{
		std::lock_guard<std::mutex> lock(terrain_edits_mutex);
		terrain_edits.clear();

		int idx = 1;
		for (int i = 0; i < count; ++i) {
			int x = static_cast<int>(data[idx++]);
			int y = static_cast<int>(data[idx++]);
			int z = static_cast<int>(data[idx++]);
			float delta = data[idx++];

			uint64_t key = pack_edit_key(x, y, z);
			terrain_edits[key] = delta;
		}
	}

	log_message(String("Loaded {0} terrain edits").format(Array::make(count)), 1);
	// Mark all chunks dirty to apply loaded edits
	mark_all_chunks_dirty();
	// Note: Cache remains valid; edits are applied via terrain_edits map in get_terrain_density()
}

// =====================================================================

// ============================================================================
// Async Generation Stubs (for WorkerThreadPool integration)
// ============================================================================

void VoxelGenerator::_chunk_generation_task(void *userdata, uint32_t chunk_index) {
	// Static callback for WorkerThreadPool::add_native_group_task
	// userdata points to the VoxelGenerator instance
	VoxelGenerator *self = static_cast<VoxelGenerator *>(userdata);
	self->generate_chunk_mesh_internal(static_cast<int>(chunk_index));
}

void VoxelGenerator::generate_chunk_mesh_internal(int chunk_index) {
	// Check for cancellation
	if (cancel_requested.load()) {
		return;
	}

	// Per-chunk mesh generation for async operation
	Vector3i chunk_coord = index_to_chunk_coord(chunk_index);
	if (chunk_coord.x < 0) {
		return; // Invalid index
	}

	log_message(String("Async generating chunk ({0},{1},{2})").format(Array::make(chunk_coord.x, chunk_coord.y, chunk_coord.z)), 3);

	// Use effective resolution (accounts for LOD)
	int eff_resolution = get_effective_resolution();

	// Calculate chunk bounds in voxel space
	int chunk_voxels_x = chunk_size * eff_resolution;
	int chunk_voxels_y = chunk_size * eff_resolution;
	int chunk_voxels_z = chunk_size * eff_resolution;

	// Start voxel indices for this chunk
	int start_ix = chunk_coord.x * chunk_voxels_x;
	int start_iy = chunk_coord.y * chunk_voxels_y;
	int start_iz = chunk_coord.z * chunk_voxels_z;

	// Physical extent of the entire world
	float physical_extent_x = static_cast<float>(std::max(1, world_size.x) * std::max(1, chunk_size));
	float physical_extent_y = static_cast<float>(std::max(1, world_size.y) * std::max(1, chunk_size));
	float physical_extent_z = static_cast<float>(std::max(1, world_size.z) * std::max(1, chunk_size));
	Vector3 physical_extent = Vector3(physical_extent_x, physical_extent_y, physical_extent_z);

	// Voxel size adjusted for effective resolution
	Vector3 eff_voxel_size = Vector3(1.0f, 1.0f, 1.0f) / static_cast<float>(std::max(1, eff_resolution));

	// Prepare mesh data arrays
	ChunkMeshData mesh_data;
	mesh_data.chunk_index = chunk_index;
	mesh_data.chunk_coord = chunk_coord;

	// Generate mesh using marching cubes for this chunk
	for (int local_ix = 0; local_ix < chunk_voxels_x && !cancel_requested.load(); ++local_ix) {
		for (int local_iy = 0; local_iy < chunk_voxels_y; ++local_iy) {
			for (int local_iz = 0; local_iz < chunk_voxels_z; ++local_iz) {
				int ix = start_ix + local_ix;
				int iy = start_iy + local_iy;
				int iz = start_iz + local_iz;

				// Calculate the center position of the voxel
				Vector3 center;
				center.x = -physical_extent.x * 0.5f + (ix + 0.5f) * eff_voxel_size.x;
				center.y = -physical_extent.y * 0.5f + (iy + 0.5f) * eff_voxel_size.y;
				center.z = -physical_extent.z * 0.5f + (iz + 0.5f) * eff_voxel_size.z;

				// Create marching cube vertices
				Vector<Vector3> cube_vertices = create_cube_vertices(center);
				std::vector<float> cube_values = get_cube_values_cached(ix, iy, iz);

				int lookup_index = get_lookup_index(cube_values, cutoff);
				const auto &marching_triangles = Constants::get_marching_triangles();

				if (lookup_index >= marching_triangles.size()) {
					continue;
				}

				std::vector<int> triangles(marching_triangles[lookup_index].begin(), marching_triangles[lookup_index].end());

				// Biome id for texturing (<= 16 biomes supported; clamp to [0,15])
				int biome_index_for_textures = 0;
				if (biome_generator.is_valid()) {
					biome_index_for_textures = CLAMP(biome_generator->get_biome_index_at(center.x, center.z), 0, 15);
				}
				Color custom0_color(static_cast<float>(biome_index_for_textures) / 255.0f, 0.0f, 0.0f, 0.0f);

				// Calculate color based on position or LOD visualization
				int total_voxels_x = std::max(1, world_size.x) * std::max(1, chunk_size) * eff_resolution;
				int total_voxels_y = std::max(1, world_size.y) * std::max(1, chunk_size) * eff_resolution;
				int total_voxels_z = std::max(1, world_size.z) * std::max(1, chunk_size) * eff_resolution;
				Color color;
				if (show_lod_colors) {
					color = get_lod_color(lod_level);
				} else if (biome_generator.is_valid()) {
					int biome_index = biome_generator->get_biome_index_at(center.x, center.z);
					color = get_biome_debug_color(biome_index);
				} else {
					color = Color(
							(center.x + total_voxels_x * 0.5f) / (float)total_voxels_x,
							(center.y + total_voxels_y * 0.5f) / (float)total_voxels_y,
							(center.z + total_voxels_z * 0.5f) / (float)total_voxels_z);
				}

				// Process triangles
				for (size_t index = 0; index < triangles.size(); index += 3) {
					int point_1 = triangles[index];
					if (point_1 == -1)
						continue;
					int point_2 = triangles[index + 1];
					if (point_2 == -1)
						continue;
					int point_3 = triangles[index + 2];
					if (point_3 == -1)
						continue;

					int a0 = Constants::cornerIndexAFromEdge[point_1];
					int b0 = Constants::cornerIndexBFromEdge[point_1];
					int a1 = Constants::cornerIndexAFromEdge[point_2];
					int b1 = Constants::cornerIndexBFromEdge[point_2];
					int a2 = Constants::cornerIndexAFromEdge[point_3];
					int b2 = Constants::cornerIndexBFromEdge[point_3];

					Vector3 vertex1 = interpolate(cube_vertices[a0], cube_values[a0], cube_vertices[b0], cube_values[b0]);
					Vector3 vertex2 = interpolate(cube_vertices[a1], cube_values[a1], cube_vertices[b1], cube_values[b1]);
					Vector3 vertex3 = interpolate(cube_vertices[a2], cube_values[a2], cube_vertices[b2], cube_values[b2]);

					Vector3 vector_a = vertex3 - vertex1;
					Vector3 vector_b = vertex2 - vertex1;
					Vector3 normal = vector_a.cross(vector_b).normalized();

					// Add vertices to mesh data
					mesh_data.vertices.push_back(vertex1);
					mesh_data.vertices.push_back(vertex2);
					mesh_data.vertices.push_back(vertex3);

					mesh_data.normals.push_back(normal);
					mesh_data.normals.push_back(normal);
					mesh_data.normals.push_back(normal);

					mesh_data.colors.push_back(color);
					mesh_data.colors.push_back(color);
					mesh_data.colors.push_back(color);

					mesh_data.custom0.push_back(custom0_color);
					mesh_data.custom0.push_back(custom0_color);
					mesh_data.custom0.push_back(custom0_color);
				}
			}
		}
	}

	// Queue the mesh data for main thread application
	if (!cancel_requested.load()) {
		queue_mesh_data(mesh_data);
	}

	// Mark chunk as clean after generation
	{
		std::lock_guard<std::mutex> lock(chunks_mutex);
		if (chunk_index >= 0 && chunk_index < static_cast<int>(chunk_dirty_flags.size())) {
			chunk_dirty_flags[chunk_index] = false;
		}
	}
}

// ============================================================================
// Synchronous Per-Chunk Mesh Generation (for main thread use)
// ============================================================================

void VoxelGenerator::generate_chunk_mesh_sync(int chunk_index, int override_lod) {
	// Synchronous per-chunk mesh generation for use on main thread
	// Similar to generate_chunk_mesh_internal but applies mesh directly instead of queuing
	Vector3i chunk_coord = index_to_chunk_coord(chunk_index);
	if (chunk_coord.x < 0) {
		return; // Invalid index
	}

	// Determine LOD level for this chunk
	int chunk_lod;
	if (override_lod >= 0) {
		// Use explicitly provided LOD
		chunk_lod = std::min(override_lod, 7);
	} else if (enable_distance_lod) {
		// Calculate LOD based on distance from reference position
		chunk_lod = calculate_chunk_lod(chunk_index);
	} else {
		// Use global LOD level
		chunk_lod = lod_level;
	}

	// Update chunk's current LOD level
	{
		std::lock_guard<std::mutex> lock(chunks_mutex);
		if (chunk_index >= 0 && chunk_index < static_cast<int>(chunks.size())) {
			Chunk *chunk = chunks[chunk_index];
			if (chunk && is_instance_valid(chunk)) {
				chunk->set_current_lod_level(chunk_lod);
			}
		}
	}

	log_message(String("Sync generating chunk ({0},{1},{2}) with LOD {3}").format(Array::make(chunk_coord.x, chunk_coord.y, chunk_coord.z, chunk_lod)), 3);

	// Use effective resolution for this chunk's LOD level
	int eff_resolution = get_effective_resolution_for_lod(chunk_lod);

	// Calculate chunk bounds in voxel space (for this chunk's LOD)
	int chunk_voxels_x = chunk_size * eff_resolution;
	int chunk_voxels_y = chunk_size * eff_resolution;
	int chunk_voxels_z = chunk_size * eff_resolution;

	// Physical extent of the entire world
	float physical_extent_x = static_cast<float>(std::max(1, world_size.x) * std::max(1, chunk_size));
	float physical_extent_y = static_cast<float>(std::max(1, world_size.y) * std::max(1, chunk_size));
	float physical_extent_z = static_cast<float>(std::max(1, world_size.z) * std::max(1, chunk_size));
	Vector3 physical_extent = Vector3(physical_extent_x, physical_extent_y, physical_extent_z);

	// Voxel size adjusted for this chunk's effective resolution
	Vector3 eff_voxel_size = Vector3(1.0f, 1.0f, 1.0f) / static_cast<float>(std::max(1, eff_resolution));

	// Check if we can use fast direct cache lookup (when chunk resolution matches cache resolution)
	bool use_fast_path = (eff_resolution == cache_resolution);

	// Prepare mesh data arrays
	PackedVector3Array vertices;
	PackedVector3Array normals;
	PackedColorArray colors;
	PackedColorArray custom0;

	// Generate mesh using marching cubes for this chunk
	for (int local_ix = 0; local_ix < chunk_voxels_x; ++local_ix) {
		for (int local_iy = 0; local_iy < chunk_voxels_y; ++local_iy) {
			for (int local_iz = 0; local_iz < chunk_voxels_z; ++local_iz) {
				// Calculate the center position of the voxel (world-space coordinates)
				Vector3 center;
				center.x = -physical_extent.x * 0.5f + (chunk_coord.x * chunk_size + (local_ix + 0.5f) / static_cast<float>(eff_resolution));
				center.y = -physical_extent.y * 0.5f + (chunk_coord.y * chunk_size + static_cast<float>(local_iy) / static_cast<float>(eff_resolution));
				center.z = -physical_extent.z * 0.5f + (chunk_coord.z * chunk_size + (local_iz + 0.5f) / static_cast<float>(eff_resolution));

				// Create marching cube vertices using chunk's voxel size
				Vector3 half_size = eff_voxel_size * 0.5f;
				Vector<Vector3> cube_vertices = Vector<Vector3>{
					Vector3(center.x - half_size.x, center.y - half_size.y, center.z - half_size.z),
					Vector3(center.x + half_size.x, center.y - half_size.y, center.z - half_size.z),
					Vector3(center.x + half_size.x, center.y + half_size.y, center.z - half_size.z),
					Vector3(center.x - half_size.x, center.y + half_size.y, center.z - half_size.z),
					Vector3(center.x - half_size.x, center.y - half_size.y, center.z + half_size.z),
					Vector3(center.x + half_size.x, center.y - half_size.y, center.z + half_size.z),
					Vector3(center.x + half_size.x, center.y + half_size.y, center.z + half_size.z),
					Vector3(center.x - half_size.x, center.y + half_size.y, center.z + half_size.z),
				};

				// Sample density values - use fast path when resolution matches cache
				std::vector<float> cube_values;
				if (use_fast_path) {
					// Fast path: direct cache lookup (no interpolation needed)
					int ix = chunk_coord.x * chunk_size * eff_resolution + local_ix;
					int iy = chunk_coord.y * chunk_size * eff_resolution + local_iy;
					int iz = chunk_coord.z * chunk_size * eff_resolution + local_iz;
					cube_values = get_cube_values_cached(ix, iy, iz);
				} else {
					// Slow path: trilinear interpolation for mismatched resolutions
					cube_values = get_cube_values_at_world_position(center, half_size);
				}

				// DEBUG: Log first voxel of each chunk to diagnose empty meshes
				if (local_ix == 0 && local_iy == 0 && local_iz == 0) {
					log_message(String("Chunk ({0},{1},{2}): first voxel center=({3:.2f},{4:.2f},{5:.2f}) density=[{6:.2f},{7:.2f},{8:.2f},{9:.2f}] cutoff={10:.2f}")
										.format(Array::make(chunk_coord.x, chunk_coord.y, chunk_coord.z,
												center.x, center.y, center.z,
												cube_values[0], cube_values[1], cube_values[2], cube_values[3],
												cutoff)),
							3);
				}

				int lookup_index = get_lookup_index(cube_values, cutoff);
				const auto &marching_triangles = Constants::get_marching_triangles();

				if (lookup_index >= marching_triangles.size()) {
					continue;
				}

				std::vector<int> triangles(marching_triangles[lookup_index].begin(), marching_triangles[lookup_index].end());

				// Biome id for texturing (<= 16 biomes supported; clamp to [0,15])
				int biome_index_for_textures = 0;
				if (biome_generator.is_valid()) {
					biome_index_for_textures = CLAMP(biome_generator->get_biome_index_at(center.x, center.z), 0, 15);
				}
				Color custom0_color(static_cast<float>(biome_index_for_textures) / 255.0f, 0.0f, 0.0f, 0.0f);

				// Calculate color based on biome or world position for visualization
				Color color;
				if (show_lod_colors) {
					color = get_lod_color(chunk_lod);
				} else if (biome_generator.is_valid()) {
					int biome_index = biome_generator->get_biome_index_at(center.x, center.z);
					color = get_biome_debug_color(biome_index);
				} else {
					// Normalize world position to 0-1 range for color fallback
					color = Color(
							(center.x + physical_extent.x * 0.5f) / physical_extent.x,
							(center.y + physical_extent.y * 0.5f) / physical_extent.y,
							(center.z + physical_extent.z * 0.5f) / physical_extent.z);
				}

				// Process triangles
				for (size_t index = 0; index < triangles.size(); index += 3) {
					int point_1 = triangles[index];
					if (point_1 == -1)
						continue;
					int point_2 = triangles[index + 1];
					if (point_2 == -1)
						continue;
					int point_3 = triangles[index + 2];
					if (point_3 == -1)
						continue;

					int a0 = Constants::cornerIndexAFromEdge[point_1];
					int b0 = Constants::cornerIndexBFromEdge[point_1];
					int a1 = Constants::cornerIndexAFromEdge[point_2];
					int b1 = Constants::cornerIndexBFromEdge[point_2];
					int a2 = Constants::cornerIndexAFromEdge[point_3];
					int b2 = Constants::cornerIndexBFromEdge[point_3];

					Vector3 vertex1 = interpolate(cube_vertices[a0], cube_values[a0], cube_vertices[b0], cube_values[b0]);
					Vector3 vertex2 = interpolate(cube_vertices[a1], cube_values[a1], cube_vertices[b1], cube_values[b1]);
					Vector3 vertex3 = interpolate(cube_vertices[a2], cube_values[a2], cube_vertices[b2], cube_values[b2]);

					Vector3 vector_a = vertex3 - vertex1;
					Vector3 vector_b = vertex2 - vertex1;
					Vector3 normal = vector_a.cross(vector_b).normalized();

					// Add vertices to mesh data
					vertices.push_back(vertex1);
					vertices.push_back(vertex2);
					vertices.push_back(vertex3);

					normals.push_back(normal);
					normals.push_back(normal);
					normals.push_back(normal);

					colors.push_back(color);
					colors.push_back(color);
					colors.push_back(color);

					custom0.push_back(custom0_color);
					custom0.push_back(custom0_color);
					custom0.push_back(custom0_color);
				}
			}
		}
	}

	// Apply mesh directly to chunk (main thread only)
	if (chunk_index >= 0 && chunk_index < static_cast<int>(chunks.size())) {
		Chunk *chunk = chunks[chunk_index];
		if (chunk && is_instance_valid(chunk)) {
			chunk->apply_mesh_data(vertices, normals, colors, custom0);
		}
	}

	// Mark chunk as clean after generation
	{
		std::lock_guard<std::mutex> lock(chunks_mutex);
		if (chunk_index >= 0 && chunk_index < static_cast<int>(chunk_dirty_flags.size())) {
			chunk_dirty_flags[chunk_index] = false;
		}
	}
}

// ============================================================================
// Heightmap Cache Implementation
// ============================================================================

void VoxelGenerator::build_heightmap_cache() {
	// Use effective resolution for heightmap
	int eff_resolution = get_effective_resolution();

	// Calculate heightmap dimensions (X-Z plane)
	heightmap_size_x = std::max(1, world_size.x) * std::max(1, chunk_size) * eff_resolution;
	heightmap_size_z = std::max(1, world_size.z) * std::max(1, chunk_size) * eff_resolution;

	size_t total_size = static_cast<size_t>(heightmap_size_x) * heightmap_size_z;

	log_message(String("Building heightmap cache: {0}x{1} = {2} entries")
						.format(Array::make(heightmap_size_x, heightmap_size_z, static_cast<int64_t>(total_size))),
			2);

	heightmap_cache.resize(total_size);

	// Physical extent
	float physical_extent_x = static_cast<float>(std::max(1, world_size.x) * std::max(1, chunk_size));
	float physical_extent_z = static_cast<float>(std::max(1, world_size.z) * std::max(1, chunk_size));

	// Voxel size adjusted for effective resolution
	float eff_voxel_size = 1.0f / static_cast<float>(std::max(1, eff_resolution));

	// Sample terrain noise to build heightmap
	for (int iz = 0; iz < heightmap_size_z; ++iz) {
		for (int ix = 0; ix < heightmap_size_x; ++ix) {
			// Convert index to world X-Z position (center of voxel column)
			float world_x = -physical_extent_x * 0.5f + (ix + 0.5f) * eff_voxel_size;
			float world_z = -physical_extent_z * 0.5f + (iz + 0.5f) * eff_voxel_size;

			// Calculate terrain height using 2D noise
			float height = terrain_height;
			if (terrain_noise.is_valid()) {
				float noise_value = terrain_noise->get_noise_2d(world_x, world_z);
				height += noise_value * terrain_amplitude;
			}

			int index = heightmap_cache_index(ix, iz);
			heightmap_cache[index] = height;
		}
	}

	log_message("Heightmap cache built successfully", 2);
}

void VoxelGenerator::clear_heightmap_cache() {
	heightmap_cache.clear();
	heightmap_cache.shrink_to_fit();
	heightmap_size_x = 0;
	heightmap_size_z = 0;
	log_message("Heightmap cache cleared", 2);
}

float VoxelGenerator::get_height_at(float fx, float fz) const {
	// Bounds check
	if (heightmap_cache.empty() ||
			fx < 0 || fx >= heightmap_size_x ||
			fz < 0 || fz >= heightmap_size_z) {
		// Fallback to direct calculation
		int eff_resolution = get_effective_resolution();
		float eff_voxel_size = 1.0f / static_cast<float>(std::max(1, eff_resolution));
		float physical_extent_x = static_cast<float>(std::max(1, world_size.x) * std::max(1, chunk_size));
		float physical_extent_z = static_cast<float>(std::max(1, world_size.z) * std::max(1, chunk_size));

		float world_x = -physical_extent_x * 0.5f + (fx + 0.5f) * eff_voxel_size;
		float world_z = -physical_extent_z * 0.5f + (fz + 0.5f) * eff_voxel_size;

		float height = terrain_height;
		if (terrain_noise.is_valid()) {
			height += terrain_noise->get_noise_2d(world_x, world_z) * terrain_amplitude;
		}
		return height;
	}

	return heightmap_cache[heightmap_cache_index(fx, fz)];
}

bool VoxelGenerator::heightmap_needs_rebuild() const {
	// Heightmap needs rebuild if cache is empty or dimensions changed
	int eff_resolution = get_effective_resolution();
	int expected_x = std::max(1, world_size.x) * std::max(1, chunk_size) * eff_resolution;
	int expected_z = std::max(1, world_size.z) * std::max(1, chunk_size) * eff_resolution;

	return heightmap_cache.empty() ||
			heightmap_size_x != expected_x ||
			heightmap_size_z != expected_z;
}

// ============================================================================
// Async Generation Property Getters/Setters
// ============================================================================

void VoxelGenerator::set_max_chunks_per_frame(int value) {
	max_chunks_per_frame = CLAMP(value, 1, 32);
	log_message(String("Max chunks per frame set to: {0}").format(Array::make(max_chunks_per_frame)), 2);
}

int VoxelGenerator::get_max_chunks_per_frame() const {
	return max_chunks_per_frame;
}

void VoxelGenerator::set_signal_every_n_chunks(int value) {
	signal_every_n_chunks = CLAMP(value, 1, 64);
	log_message(String("Signal every N chunks set to: {0}").format(Array::make(signal_every_n_chunks)), 2);
}

int VoxelGenerator::get_signal_every_n_chunks() const {
	return signal_every_n_chunks;
}

bool VoxelGenerator::is_generating() const {
	return generation_in_progress.load();
}

// ============================================================================
// Async Generation Implementation
// ============================================================================

void VoxelGenerator::generate_async() {
	log_message("generate_async() called", 2);

	// If already generating, cancel and restart
	if (generation_in_progress.load()) {
		log_message("Async generation already in progress, canceling and restarting", 1);
		cancel_generation();
	}

	// Mark generation as in progress
	generation_in_progress.store(true);
	cancel_requested.store(false);
	chunks_completed.store(0);
	last_signal_count = 0;

	// Ensure density cache is built before starting worker tasks
	if (!terrain_noise.is_valid()) {
		terrain_noise.instantiate();
		terrain_noise->set_period(50.0f);
		terrain_noise->set_octaves(4);
		terrain_noise->set_persistence(0.5f);
		terrain_noise->set_lacunarity(2.0f);
		terrain_noise->set_seed(seeder);
	}
	if (!detail_noise.is_valid()) {
		detail_noise.instantiate();
		detail_noise->set_period(10.0f);
		detail_noise->set_octaves(3);
		detail_noise->set_persistence(0.6f);
		detail_noise->set_lacunarity(2.5f);
		detail_noise->set_seed(seeder + 1);
	}

	// Ensure vertical extent matches biome heights before building caches
	ensure_vertical_extent_for_biomes();
	recalculate_voxel_scale();

	// Calculate total chunks AFTER vertical extent expansion
	total_chunks = world_size.x * world_size.y * world_size.z;
	if (total_chunks == 0) {
		log_message("No chunks to generate (world_size is zero)", 1);
		generation_in_progress.store(false);
		emit_signal("generation_complete");
		return;
	}

	log_message(String("Starting async generation of {0} chunks").format(Array::make(total_chunks)), 2);

	// Ensure chunk node list matches the expected total_chunks. Recreate when mismatched.
	if (static_cast<int>(chunks.size()) != total_chunks) {
		log_message(String("Chunk node count ({0}) does not match expected total_chunks ({1}), recreating chunk nodes")
							.format(Array::make(static_cast<int>(chunks.size()), total_chunks)),
				1);
		create_chunks();
	}

	// Bake procedural features ahead of density sampling
	prepare_procedural_features();

	if (generation_mode == VOXELS_FIRST) {
		build_density_cache();
	} else {
		build_heightmap_cache();
		build_density_cache();
	}

	// Clear any old mesh data
	clear_pending_meshes();
	for (Chunk *chunk : chunks) {
		if (chunk) {
			chunk->clear_mesh();
		}
	}

	// Start worker thread pool tasks
	async_task_group = WorkerThreadPool::get_singleton()->add_native_group_task(
			&VoxelGenerator::_chunk_generation_task,
			this,
			total_chunks,
			-1, // Use default thread distribution
			true, // High priority
			String("VoxelChunkGeneration"));

	// Enable _process to poll for completed meshes
	set_process(true);

	log_message(String("Async generation started with task group {0}").format(Array::make(static_cast<int64_t>(async_task_group))), 2);
}

void VoxelGenerator::cancel_generation() {
	if (!generation_in_progress.load()) {
		return;
	}

	log_message("Canceling async generation", 1);
	cancel_requested.store(true);

	// Wait for task group to complete
	if (async_task_group != 0) {
		WorkerThreadPool::get_singleton()->wait_for_group_task_completion(async_task_group);
		async_task_group = 0;
	}

	// Clear pending mesh queue
	clear_pending_meshes();

	// Reset state
	generation_in_progress.store(false);
	cancel_requested.store(false);
	chunks_completed.store(0);
	total_chunks = 0;

	// Disable _process
	set_process(false);

	log_message("Async generation canceled", 1);
}

void VoxelGenerator::rebuild_debug_visualizations() {
	// Remove old visualization nodes
	for (int i = 0; i < get_child_count(); ++i) {
		Node *child = get_child(i);
		StringName child_name = child->get_name();
		if (child_name == StringName("MeshInstanceCenters") ||
				child_name == StringName("MeshInstanceVoxelGrid") ||
				child_name == StringName("MeshInstanceChunkGrid")) {
			child->queue_free();
		}
	}

	// Physical extent for visualizations
	float physical_extent_x = static_cast<float>(std::max(1, world_size.x) * std::max(1, chunk_size));
	float physical_extent_y = static_cast<float>(std::max(1, world_size.y) * std::max(1, chunk_size));
	float physical_extent_z = static_cast<float>(std::max(1, world_size.z) * std::max(1, chunk_size));
	Vector3 physical_extent = Vector3(physical_extent_x, physical_extent_y, physical_extent_z);

	// Use effective resolution for voxel grid
	int eff_resolution = get_effective_resolution();
	Vector3 eff_voxel_size = Vector3(1.0f, 1.0f, 1.0f) / static_cast<float>(std::max(1, eff_resolution));

	// Centers visualization (debug points) - empty for now
	Ref<ImmediateMesh> mesh_centers;
	mesh_centers.instantiate();
	MeshInstance3D *mi_centers = memnew(MeshInstance3D);
	mi_centers->set_name("MeshInstanceCenters");
	mi_centers->set_visible(show_centers);
	mi_centers->set_mesh(mesh_centers);
	add_child(mi_centers);

	// Voxel grid visualization
	Ref<ImmediateMesh> mesh_cubes;
	mesh_cubes.instantiate();

	if (show_voxel_grid) {
		mesh_cubes->surface_begin(Mesh::PRIMITIVE_LINES);

		int total_voxels_x = std::max(1, world_size.x) * std::max(1, chunk_size) * eff_resolution;
		int total_voxels_y = std::max(1, world_size.y) * std::max(1, chunk_size) * eff_resolution;
		int total_voxels_z = std::max(1, world_size.z) * std::max(1, chunk_size) * eff_resolution;

		Color grid_color(0.5f, 0.5f, 0.5f, 1.0f);
		int cubes_vertex_count = 0;
		int skipped_voxels = 0;
		float eff_surface_band = get_effective_surface_band();

		if (generation_mode == HEIGHTMAP_FIRST) {
			for (int ix = 0; ix < total_voxels_x; ++ix) {
				for (int iz = 0; iz < total_voxels_z; ++iz) {
					float terrain_height_at_xz = get_height_at(ix, iz);
					float world_y_center = terrain_height_at_xz;
					float y_min_world = world_y_center - eff_surface_band;
					float y_max_world = world_y_center + eff_surface_band;

					int iy_min = std::max(0, static_cast<int>((y_min_world + physical_extent.y * 0.5f) / eff_voxel_size.y));
					int iy_max = std::min(total_voxels_y - 1, static_cast<int>((y_max_world + physical_extent.y * 0.5f) / eff_voxel_size.y));

					for (int iy = iy_min; iy <= iy_max; ++iy) {
						if (cubes_vertex_count >= Constants::MAX_VERTICES || vertex_limit)
							break;

						Vector3 center;
						center.x = -physical_extent.x * 0.5f + (ix + 0.5f) * eff_voxel_size.x;
						center.y = -physical_extent.y * 0.5f + (iy + 0.5f) * eff_voxel_size.y;
						center.z = -physical_extent.z * 0.5f + (iz + 0.5f) * eff_voxel_size.z;

						float center_value = get_terrain_density(center);
						if (center_value >= cutoff)
							continue;

						Vector<Vector3> cube_vertices = create_cube_vertices(center);
						const int edges[12][2] = {
							{ 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 },
							{ 0, 4 }, { 2, 6 }, { 5, 6 }, { 5, 4 },
							{ 5, 1 }, { 6, 7 }, { 4, 7 }, { 3, 7 }
						};
						for (int e = 0; e < 12; ++e) {
							mesh_cubes->surface_set_color(grid_color);
							mesh_cubes->surface_add_vertex(cube_vertices[edges[e][0]]);
							mesh_cubes->surface_add_vertex(cube_vertices[edges[e][1]]);
						}
						cubes_vertex_count += 24;
					}
					skipped_voxels += (total_voxels_y - (iy_max - iy_min + 1));
					if (cubes_vertex_count >= Constants::MAX_VERTICES || vertex_limit)
						break;
				}
				if (cubes_vertex_count >= Constants::MAX_VERTICES || vertex_limit)
					break;
			}

			// Log efficiency stats
			int total_possible = total_voxels_x * total_voxels_y * total_voxels_z;
			float efficiency = (total_possible > 0) ? (100.0f * skipped_voxels / total_possible) : 0.0f;
			log_message(String("Voxel grid (HEIGHTMAP): {0} vertices, {1}% voxels skipped due to surface band")
								.format(Array::make(cubes_vertex_count, int(efficiency))),
					2);
		} else {
			for (int ix = 0; ix < total_voxels_x; ++ix) {
				for (int iy = 0; iy < total_voxels_y; ++iy) {
					for (int iz = 0; iz < total_voxels_z; ++iz) {
						if (cubes_vertex_count >= Constants::MAX_VERTICES || vertex_limit)
							break;

						Vector3 center;
						center.x = -physical_extent.x * 0.5f + (ix + 0.5f) * eff_voxel_size.x;
						center.y = -physical_extent.y * 0.5f + (iy + 0.5f) * eff_voxel_size.y;
						center.z = -physical_extent.z * 0.5f + (iz + 0.5f) * eff_voxel_size.z;

						float center_value = get_terrain_density(center);
						if (center_value >= cutoff)
							continue;

						Vector<Vector3> cube_vertices = create_cube_vertices(center);
						const int edges[12][2] = {
							{ 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 },
							{ 0, 4 }, { 2, 6 }, { 5, 6 }, { 5, 4 },
							{ 5, 1 }, { 6, 7 }, { 4, 7 }, { 3, 7 }
						};
						for (int e = 0; e < 12; ++e) {
							mesh_cubes->surface_set_color(grid_color);
							mesh_cubes->surface_add_vertex(cube_vertices[edges[e][0]]);
							mesh_cubes->surface_add_vertex(cube_vertices[edges[e][1]]);
						}
						cubes_vertex_count += 24;
					}
					if (cubes_vertex_count >= Constants::MAX_VERTICES || vertex_limit)
						break;
				}
				if (cubes_vertex_count >= Constants::MAX_VERTICES || vertex_limit)
					break;
			}
			log_message(String("Voxel grid (VOXELS_FIRST): {0} vertices").format(Array::make(cubes_vertex_count)), 2);
		}

		if (cubes_vertex_count > 0) {
			mesh_cubes->surface_end();
			Ref<StandardMaterial3D> material_cubes;
			material_cubes.instantiate();
			material_cubes->set_flag(godot::BaseMaterial3D::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
			material_cubes->set_shading_mode(BaseMaterial3D::SHADING_MODE_UNSHADED);
			mesh_cubes->surface_set_material(0, material_cubes);
		}
	}

	MeshInstance3D *mi_cubes = memnew(MeshInstance3D);
	mi_cubes->set_name("MeshInstanceVoxelGrid");
	mi_cubes->set_visible(show_voxel_grid);
	mi_cubes->set_mesh(mesh_cubes);
	add_child(mi_cubes);

	// Chunk grid visualization
	Ref<ImmediateMesh> mesh_chunk_grid;
	mesh_chunk_grid.instantiate();
	mesh_chunk_grid->surface_begin(Mesh::PRIMITIVE_TRIANGLES);

	int chunk_grid_vertex_count = 0;
	float line_thickness = 0.1f;
	Color chunk_grid_color(1.0f, 0.0f, 0.0f);

	float half_extent_x = physical_extent.x * 0.5f;
	float half_extent_y = physical_extent.y * 0.5f;
	float half_extent_z = physical_extent.z * 0.5f;

	auto draw_thick_line = [&](Vector3 start, Vector3 end, Vector3 up) {
		Vector3 dir = (end - start).normalized();
		Vector3 side = dir.cross(up).normalized() * line_thickness * 0.5f;

		Vector3 v0 = start - side;
		Vector3 v1 = start + side;
		Vector3 v2 = end + side;
		Vector3 v3 = end - side;

		mesh_chunk_grid->surface_set_color(chunk_grid_color);
		mesh_chunk_grid->surface_add_vertex(v0);
		mesh_chunk_grid->surface_add_vertex(v1);
		mesh_chunk_grid->surface_add_vertex(v2);

		mesh_chunk_grid->surface_set_color(chunk_grid_color);
		mesh_chunk_grid->surface_add_vertex(v0);
		mesh_chunk_grid->surface_add_vertex(v2);
		mesh_chunk_grid->surface_add_vertex(v3);

		chunk_grid_vertex_count += 6;
	};

	for (int cx = 0; cx <= world_size.x; ++cx) {
		for (int cy = 0; cy <= world_size.y; ++cy) {
			for (int cz = 0; cz <= world_size.z; ++cz) {
				float x = -half_extent_x + cx * chunk_size;
				float y = -half_extent_y + cy * chunk_size;
				float z = -half_extent_z + cz * chunk_size;

				if (cx < world_size.x) {
					Vector3 start(x, y, z);
					Vector3 end(x + chunk_size, y, z);
					draw_thick_line(start, end, Vector3(0, 1, 0));
				}

				if (cy < world_size.y) {
					Vector3 start(x, y, z);
					Vector3 end(x, y + chunk_size, z);
					draw_thick_line(start, end, Vector3(1, 0, 0));
				}

				if (cz < world_size.z) {
					Vector3 start(x, y, z);
					Vector3 end(x, y, z + chunk_size);
					draw_thick_line(start, end, Vector3(0, 1, 0));
				}
			}
		}
	}

	if (chunk_grid_vertex_count > 0) {
		mesh_chunk_grid->surface_end();

		Ref<StandardMaterial3D> material_chunk_grid;
		material_chunk_grid.instantiate();
		material_chunk_grid->set_albedo(Color(1.0f, 0.0f, 0.0f));
		material_chunk_grid->set_shading_mode(BaseMaterial3D::SHADING_MODE_UNSHADED);
		material_chunk_grid->set_flag(BaseMaterial3D::FLAG_DISABLE_DEPTH_TEST, false);

		mesh_chunk_grid->surface_set_material(0, material_chunk_grid);

		MeshInstance3D *mi_chunk_grid = memnew(MeshInstance3D);
		mi_chunk_grid->set_name("MeshInstanceChunkGrid");
		mi_chunk_grid->set_visible(show_chunk_grid);
		mi_chunk_grid->set_mesh(mesh_chunk_grid);
		add_child(mi_chunk_grid);
	}

	if (visualize_noise_values) {
		visualize_noise_field();
	}
}

void VoxelGenerator::_process(double delta) {
	// Only process if we're doing async generation
	if (!generation_in_progress.load()) {
		set_process(false);
		return;
	}

	// Apply pending meshes (up to max_chunks_per_frame)
	apply_pending_meshes();

	// Check if all tasks completed
	if (async_task_group != 0) {
		bool tasks_done = WorkerThreadPool::get_singleton()->is_group_task_completed(async_task_group);
		if (tasks_done) {
			// Wait for final completion to clean up
			WorkerThreadPool::get_singleton()->wait_for_group_task_completion(async_task_group);
			async_task_group = 0;

			// Apply any remaining meshes
			while (!pending_mesh_queue.empty()) {
				apply_pending_meshes();
			}

			// Mark cache as valid for reuse (don't clear it)
			cache_is_valid = true;

			// Rebuild debug visualizations (voxel grid, chunk grid) after async generation completes
			rebuild_debug_visualizations();

			// Mark complete
			generation_in_progress.store(false);
			set_process(false);

			emit_signal("generation_complete");
			log_message(String("Async generation complete: {0} chunks").format(Array::make(chunks_completed.load())), 1);
		}
	}
}

void VoxelGenerator::queue_mesh_data(const ChunkMeshData &mesh_data) {
	std::lock_guard<std::mutex> lock(mesh_queue_mutex);
	pending_mesh_queue.push(mesh_data);
}

void VoxelGenerator::apply_pending_meshes() {
	int applied = 0;

	while (applied < max_chunks_per_frame) {
		ChunkMeshData mesh_data;
		{
			std::lock_guard<std::mutex> lock(mesh_queue_mutex);
			if (pending_mesh_queue.empty()) {
				break;
			}
			mesh_data = std::move(pending_mesh_queue.front());
			pending_mesh_queue.pop();
		}

		// Apply mesh to chunk (main thread only)
		if (mesh_data.chunk_index >= 0 && mesh_data.chunk_index < static_cast<int>(chunks.size())) {
			Chunk *chunk = chunks[mesh_data.chunk_index];
			if (chunk && is_instance_valid(chunk)) {
				chunk->apply_mesh_data(mesh_data.vertices, mesh_data.normals, mesh_data.colors, mesh_data.custom0);

				// Emit chunk_ready signal
				emit_signal("chunk_ready", mesh_data.chunk_index, mesh_data.chunk_coord);

				int completed = chunks_completed.fetch_add(1) + 1;

				// Emit progress signal every N chunks
				if (completed - last_signal_count >= signal_every_n_chunks) {
					emit_signal("generation_progress", completed, total_chunks);
					last_signal_count = completed;
				}
			}
		}

		applied++;
	}
}

void VoxelGenerator::clear_pending_meshes() {
	std::lock_guard<std::mutex> lock(mesh_queue_mutex);
	while (!pending_mesh_queue.empty()) {
		pending_mesh_queue.pop();
	}
}

} // namespace voxel_engine
