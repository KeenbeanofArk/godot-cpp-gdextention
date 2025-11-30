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
#include "generators/NoiseGenerator.h"

#include <algorithm>
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

	ClassDB::bind_method(D_METHOD("set_world_size", "value"), &VoxelGenerator::set_world_size);
	ClassDB::bind_method(D_METHOD("get_world_size"), &VoxelGenerator::get_world_size);
	ClassDB::bind_method(D_METHOD("set_chunk_size", "value"), &VoxelGenerator::set_chunk_size);
	ClassDB::bind_method(D_METHOD("get_chunk_size"), &VoxelGenerator::get_chunk_size);

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

	ClassDB::bind_method(D_METHOD("reset"), &VoxelGenerator::reset);

	// Bind dirty flag / incremental update methods
	ClassDB::bind_method(D_METHOD("mark_chunk_dirty", "chunk_coord"), &VoxelGenerator::mark_chunk_dirty);
	ClassDB::bind_method(D_METHOD("mark_all_chunks_dirty"), &VoxelGenerator::mark_all_chunks_dirty);
	ClassDB::bind_method(D_METHOD("is_chunk_dirty", "chunk_coord"), &VoxelGenerator::is_chunk_dirty);
	ClassDB::bind_method(D_METHOD("regenerate_dirty_chunks"), &VoxelGenerator::regenerate_dirty_chunks);
	ClassDB::bind_method(D_METHOD("invalidate_density_region", "min_voxel", "max_voxel"), &VoxelGenerator::invalidate_density_region);

	ClassDB::bind_method(D_METHOD("is_object_binding_set_by_parent_constructor"), &VoxelGenerator::is_object_binding_set_by_parent_constructor);

	ADD_GROUP("voxel_generator", "voxel_generator_");
	ADD_SUBGROUP("voxel_generator_", "World Settings");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3I, "world_size", PROPERTY_HINT_RANGE, "1,100,1"), "set_world_size", "get_world_size");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_generate"), "set_auto_generate", "get_auto_generate");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "chunk_size", PROPERTY_HINT_RANGE, "8,64,8"), "set_chunk_size", "get_chunk_size");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "resolution", PROPERTY_HINT_RANGE, "1,10,1"), "set_resolution", "get_resolution");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "cutoff", PROPERTY_HINT_RANGE, "-1,1,0.01"), "set_cutoff", "get_cutoff");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "seeder", PROPERTY_HINT_RANGE, "0,1000000,1"), "set_seeder", "get_seeder");

	ADD_GROUP("Terrain Settings", "terrain_");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "terrain_noise", PROPERTY_HINT_RESOURCE_TYPE, "NoiseGenerator"), "set_terrain_noise", "get_terrain_noise");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "detail_noise", PROPERTY_HINT_RESOURCE_TYPE, "NoiseGenerator"), "set_detail_noise", "get_detail_noise");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "terrain_height", PROPERTY_HINT_RANGE, "-100,100,0.5"), "set_terrain_height", "get_terrain_height");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "terrain_amplitude", PROPERTY_HINT_RANGE, "0,100,0.5"), "set_terrain_amplitude", "get_terrain_amplitude");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "rock_influence", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_rock_influence", "get_rock_influence");

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
}

bool VoxelGenerator::has_object_instance_binding() const {
	return internal::gdextension_interface_object_get_instance_binding(_owner, internal::token, nullptr);
}

VoxelGenerator::VoxelGenerator() :
		object_instance_binding_set_by_parent_constructor(false) {
	// Initialize default values only - don't call Godot API functions yet
	resolution = 1;
	cutoff = 0.0f;
	show_centers = false;
	show_voxel_grid = false;
	show_chunk_grid = false;
	seeder = 1240;
	auto_generate = true;
	vertex_limit = false;

	// Terrain generation defaults
	rock_influence = 0.3f;
	terrain_height = 4.0f;
	terrain_amplitude = 8.0f;

	// Create default terrain noise generator (smooth rolling hills)
	/*terrain_noise.instantiate();
	terrain_noise->set_period(50.0f);
	terrain_noise->set_octaves(4);
	terrain_noise->set_persistence(0.5f);
	terrain_noise->set_lacunarity(2.0f);
	terrain_noise->set_seed(seeder);

	// Create default detail noise generator (rocky detail)
	detail_noise.instantiate();
	detail_noise->set_period(10.0f);
	detail_noise->set_octaves(3);
	detail_noise->set_persistence(0.6f);
	detail_noise->set_lacunarity(2.5f);
	detail_noise->set_seed(seeder + 1);*/

	// Note: terrain_noise and detail_noise are created lazily in generate()
	// to avoid Godot's "Instantiated X used as default value" warning
}

VoxelGenerator::~VoxelGenerator() {
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
	// Voxel size is always 1x1x1 - world dimensions equal voxel count
	voxel_size = Vector3(1.0f, 1.0f, 1.0f);
	log_message(String("Voxel size set to fixed: {0}").format(Array::make(voxel_size)), 2);
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
			set_process(false);
			set_physics_process(false);

			remove_children();
			randomize_seed();

			if (auto_generate) {
				generate();
				log_message("VoxelGenerator is ready and auto generation is enabled. Voxel grid generated.", 1);
				create_chunks();
			} else {
				log_message("VoxelGenerator is ready, but auto generation is disabled. Call generate() to create the voxel grid.", 1);
			}
			break;
		}
		case NOTIFICATION_PREDELETE:
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
	randomize_seed();
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
	resolution = value;
	log_message(String("Resolution set to: {0}").format(Array::make(resolution)), 2);
	// Resolution only affects noise sampling frequency, not voxel grid density
	// So we don't need to recalculate voxel scale, just regenerate with new noise detail
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
	if (value) {
		randomize_seed();
		log_message(String("Randomizer enabled. New seed: {0}").format(Array::make(seeder)), 2);
		if (auto_generate)
			generate();
	}
}

bool VoxelGenerator::get_randomizer() const {
	return true; // This is a placeholder, as the randomizer is always enabled in this implementation.
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

	// Initialize dirty flags if needed
	{
		std::lock_guard<std::mutex> lock(chunks_mutex);
		int total_chunks = world_size.x * world_size.y * world_size.z;
		if (chunk_dirty_flags.size() != static_cast<size_t>(total_chunks)) {
			chunk_dirty_flags.resize(total_chunks, false);
		}
	}

	// Build the density cache for fast lookups during marching cubes
	build_density_cache();

	// # Create centers mesh
	Ref<ImmediateMesh> mesh_centers;
	mesh_centers.instantiate();
	mesh_centers->surface_begin(Mesh::PRIMITIVE_POINTS);

	// # Create cubes mesh
	Ref<ImmediateMesh> mesh_cubes;
	mesh_cubes.instantiate();
	mesh_cubes->surface_begin(Mesh::PRIMITIVE_LINES);

	// # Create triangles mesh
	Ref<ImmediateMesh> mesh_triangles;
	mesh_triangles.instantiate();
	mesh_triangles->surface_begin(Mesh::PRIMITIVE_TRIANGLES);

	int centers_vertex_count = 0;
	int cubes_vertex_count = 0;

	log_message("Meshes created", 2);

	// Compute total voxels per axis - physical extent equals total voxels (voxel_size is 1x1x1)
	int total_voxels_x = std::max(1, world_size.x) * std::max(1, chunk_size);
	int total_voxels_y = std::max(1, world_size.y) * std::max(1, chunk_size);
	int total_voxels_z = std::max(1, world_size.z) * std::max(1, chunk_size);

	// Physical extent equals total voxels since voxel_size is 1x1x1
	Vector3 physical_extent = Vector3((float)total_voxels_x, (float)total_voxels_y, (float)total_voxels_z);

	int total_cubes = total_voxels_x * total_voxels_y * total_voxels_z;
	int current_cube = 0;
	int triangle_count = 0;

	log_message(String("Total voxels: {0}x{1}x{2} = {3}").format(Array::make(total_voxels_x, total_voxels_y, total_voxels_z, total_cubes)), 2);

	int non_empty_lookup_count = 0;
	std::vector<int> sample_lookup_indices;
	std::vector<std::vector<float>> sample_cube_values;

	int vertex_count = 0;

	for (int ix = 0; ix < total_voxels_x; ++ix) {
		for (int iy = 0; iy < total_voxels_y; ++iy) {
			for (int iz = 0; iz < total_voxels_z; ++iz) {
				current_cube++;

				// Check vertex limits before adding vertices
				if (vertex_count >= Constants::MAX_VERTICES || vertex_limit) {
					log_message("Vertex limit reached, stopping generation", 1);
					break; // Stop processing if vertex limit is reached
				}

#pragma region Debug
				if (debug_mode && debug_verbosity >= 3) {
					// Progress update for very verbose mode
					if (current_cube % 1000 == 0 || current_cube == total_cubes) {
						log_message(String("Processing cube {0}/{1} ({2}%)")
											.format(Array::make(current_cube, total_cubes, int(100.0f * current_cube / total_cubes))),
								3);
					}
				}
#pragma endregion

				// Calculate the center position of the voxel using voxel_size and physical extent
				// Centers range from -physical_extent/2 .. +physical_extent/2
				Vector3 center;
				center.x = -physical_extent.x * 0.5f + (ix + 0.5f) * voxel_size.x;
				center.y = -physical_extent.y * 0.5f + (iy + 0.5f) * voxel_size.y;
				center.z = -physical_extent.z * 0.5f + (iz + 0.5f) * voxel_size.z;

				log_message(String("Processing cube at {0},{1},{2}").format(Array::make(center.x, center.y, center.z)), 3);

				// Get the terrain density at the center position
				float center_value = get_terrain_density(center);
#pragma region Debug
				if (debug_mode && debug_verbosity >= 3) {
					log_message(String("  Cube at {0},{1},{2}: density={3}")
										.format(Array::make(center.x, center.y, center.z, center_value)),
							3);
				}
#pragma endregion

				// Create marching cube vertices (geometry positions)
				Vector<Vector3> cube_vertices = create_cube_vertices(center);

				// Get the terrain density at each corner of the cube using cached values
				std::vector<float> cube_values = get_cube_values_cached(ix, iy, iz);

				if (center_value < cutoff) {
					add_cubes_vertices(mesh_cubes, cube_vertices);
					// add_cubes_vertices adds 24 line-vertices (12 lines * 2 ends)
					cubes_vertex_count += 24;
				} // Get the lookup index for the current cube

				int lookup_index = get_lookup_index(cube_values, cutoff); // Bounds check to prevent crash with incomplete lookup table
				const auto &marching_triangles = Constants::get_marching_triangles();

#pragma region Debug
				if (lookup_index >= marching_triangles.size()) {
					if (debug_mode && debug_verbosity >= 2) {
						log_message(String("Warning: lookup_index {0} exceeds table size {1}, skipping cube")
											.format(Array::make(lookup_index, (int)marching_triangles.size())),
								1);
					}
					continue; // Skip this cube to prevent crash
				}
#pragma endregion

				// Get triangles
				std::vector<int> triangles(marching_triangles[lookup_index].begin(), marching_triangles[lookup_index].end());

				// Diagnostic: record non-empty lookup tables and a few sample cube values for inspection
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
					log_message(String("Cube center at {0} with value {1}").format(Array::make(center, center_value)), 3);
				};

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
					} else {
						log_message(String("Vertex limit reached, skipping additional vertices - Total vertices: {0}").format(Array::make(vertex_count)), 2);
					}
					log_message(String("Total vertices so far: {0}").format(Array::make(vertex_count)), 3);
				}
			}
		}
	}

	log_message(String("Generation completed: {0} triangles created").format(Array::make(triangle_count)), 2);

	// Diagnostic summary: print non-empty lookup counts and a few sample indices/values
	if (debug_mode && debug_verbosity >= 2) {
		std::ostringstream ss;
		ss << "Non-empty marching-triangle entries encountered: " << non_empty_lookup_count << "\n";
		ss << "Collected sample lookup indices (up to 5): ";
		for (size_t i = 0; i < sample_lookup_indices.size(); ++i) {
			ss << sample_lookup_indices[i] << (i + 1 < sample_lookup_indices.size() ? ", " : "\n");
		}
		for (size_t i = 0; i < sample_cube_values.size(); ++i) {
			ss << "Sample[" << i << "] lookup=" << sample_lookup_indices[i] << " values=[";
			for (size_t j = 0; j < sample_cube_values[i].size(); ++j) {
				ss << sample_cube_values[i][j] << (j + 1 < sample_cube_values[i].size() ? ", " : "]\n");
			}
		}
		log_message(String(ss.str().c_str()), 2);
	}

	// # End surfaces - only end surfaces that actually have vertices to avoid ImmediateMesh errors
	if (centers_vertex_count > 0) {
		mesh_centers->surface_end();
	}
	if (cubes_vertex_count > 0) {
		mesh_cubes->surface_end();
	}
	if (vertex_count > 0) {
		mesh_triangles->surface_end();
	}

	// # Create centers material
	Ref<StandardMaterial3D> material_centers;
	material_centers.instantiate();
	material_centers->set_flag(godot::BaseMaterial3D::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
	material_centers->set_shading_mode(BaseMaterial3D::SHADING_MODE_UNSHADED);
	material_centers->set_point_size(20.0);

	// # Create cubes material
	Ref<StandardMaterial3D> material_cubes;
	material_cubes.instantiate();
	material_cubes->set_flag(godot::BaseMaterial3D::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
	material_cubes->set_shading_mode(BaseMaterial3D::SHADING_MODE_UNSHADED);

	// # Create triangles material
	Ref<StandardMaterial3D> material_triangles;
	material_triangles.instantiate();
	material_triangles->set_flag(godot::BaseMaterial3D::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);

	// Only set materials for surfaces that actually exist to avoid out-of-bounds errors
	if (centers_vertex_count > 0)
		mesh_centers->surface_set_material(0, material_centers);
	if (cubes_vertex_count > 0)
		mesh_cubes->surface_set_material(0, material_cubes);
	if (vertex_count > 0)
		mesh_triangles->surface_set_material(0, material_triangles);

	// # Create mesh instance nodes and add them to the scene
	MeshInstance3D *mi_centers = memnew(MeshInstance3D);
	mi_centers->set_name("MeshInstanceCenters");
	mi_centers->set_visible(show_centers);
	mi_centers->set_mesh(mesh_centers);
	add_child(mi_centers);

	// # Create cubes mesh instance and add it to the scene (voxel grid)
	MeshInstance3D *mi_cubes = memnew(MeshInstance3D);
	mi_cubes->set_name("MeshInstanceVoxelGrid");
	mi_cubes->set_visible(show_voxel_grid);
	mi_cubes->set_mesh(mesh_cubes);
	add_child(mi_cubes);

	// # Create triangles mesh instance and add it to the scene
	MeshInstance3D *mi_triangles = memnew(MeshInstance3D);
	mi_triangles->set_mesh(mesh_triangles);
	add_child(mi_triangles);

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

	// Clear the density cache to free memory (optional: keep for incremental updates)
	clear_density_cache();

	// Mark generation complete
	generation_in_progress.store(false);

	log_message("VoxelGenerator::generate() completed", 2);
}

Vector<Vector3> VoxelGenerator::create_cube_vertices(const Vector3 &pos) {
	// Use computed voxel_size so cube dimensions follow world/chunk/resolution settings
	Vector3 half = voxel_size * 0.5f;
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
	// Calculate terrain height using 2D noise for smooth rolling hills
	float height = terrain_height;
	if (terrain_noise.is_valid()) {
		// Use 2D noise for base terrain - noise returns -1 to 1, scale by amplitude
		float noise_value = terrain_noise->get_noise_2d(pos.x, pos.z);
		height += noise_value * terrain_amplitude;
	}

	// Add rocky 3D detail using detail noise
	float rocky_detail = 0.0f;
	if (detail_noise.is_valid() && rock_influence > 0.0f) {
		rocky_detail = detail_noise->get_noise_3d(pos.x, pos.y, pos.z) * rock_influence * terrain_amplitude * 0.5f;
	}

	// Density function: negative = solid (below terrain), positive = air (above terrain)
	// pos.y - height gives us: negative when below terrain surface, positive when above
	return pos.y - height + rocky_detail;
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
	String debug_info = "VoxelGenerator Debug Information:\n";
	debug_info += String("- Debug Mode: {0}\n").format(Array::make(debug_mode));
	debug_info += String("- Debug Verbosity: {0}\n").format(Array::make(debug_verbosity));
	debug_info += String("- World Size: {0}\n").format(Array::make(world_size));
	debug_info += String("- Voxel Size: {0}\n").format(Array::make(voxel_size));
	debug_info += String("- Terrain Height: {0}\n").format(Array::make(terrain_height));
	debug_info += String("- Terrain Amplitude: {0}\n").format(Array::make(terrain_amplitude));
	debug_info += String("- Rock Influence: {0}\n").format(Array::make(rock_influence));
	debug_info += String("- Resolution: {0}\n").format(Array::make(resolution));
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

	for (float x = -half_extent_x; x < half_extent_x; x += step) {
		for (float z = -half_extent_z; z < half_extent_z; z += step) {
			// Use terrain density for visualization
			float density = get_terrain_density(Vector3(x, y_level, z));

			// Normalize density to color (blue = solid/negative, red = air/positive)
			Color color;
			if (density < cutoff) {
				// Below cutoff (solid) - blue to cyan
				float t = CLAMP((density + terrain_amplitude) / (2.0f * terrain_amplitude), 0.0f, 1.0f);
				color = Color(0, t, 1.0f);
			} else {
				// Above cutoff (air) - yellow to red
				float t = CLAMP(density / terrain_amplitude, 0.0f, 1.0f);
				color = Color(1.0f, 1.0f - t, 0);
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

	// Create new chunks properly
	for (int x = 0; x < world_size.x; x++) {
		for (int y = 0; y < world_size.y; y++) {
			for (int z = 0; z < world_size.z; z++) {
				Chunk *chunk = memnew(Chunk);
				chunk->set_name(String("Chunk_{0}_{1}_{2}").format(Array::make(x, y, z)));
				chunk->set_chunk_size(chunk_size); // Set the chunk size
				add_child(chunk); // Add to scene tree first

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
// Density Cache Implementation
// ============================================================================

void VoxelGenerator::build_density_cache() {
	// Calculate cache dimensions (all corner positions for marching cubes)
	// For a volume of size (world_size * chunk_size), we need +1 corners in each dimension
	int voxel_size_x = world_size.x * chunk_size;
	int voxel_size_y = world_size.y * chunk_size;
	int voxel_size_z = world_size.z * chunk_size;

	cache_size_x = voxel_size_x + 1;
	cache_size_y = voxel_size_y + 1;
	cache_size_z = voxel_size_z + 1;

	size_t total_size = static_cast<size_t>(cache_size_x) * cache_size_y * cache_size_z;

	log_message(String("Building density cache: {0}x{1}x{2} = {3} entries").format(Array::make(cache_size_x, cache_size_y, cache_size_z, static_cast<int64_t>(total_size))), 2);

	{
		std::unique_lock<std::shared_mutex> lock(density_cache_mutex);
		density_cache.resize(total_size);

		// Fill the cache with density values
		// Convert voxel indices to world positions for density sampling
		int total_voxels_x = world_size.x * chunk_size;
		int total_voxels_y = world_size.y * chunk_size;
		int total_voxels_z = world_size.z * chunk_size;
		Vector3 physical_extent((float)total_voxels_x, (float)total_voxels_y, (float)total_voxels_z);

		for (int iz = 0; iz < cache_size_z; ++iz) {
			for (int iy = 0; iy < cache_size_y; ++iy) {
				for (int ix = 0; ix < cache_size_x; ++ix) {
					// Convert index to world position (corner position, not center)
					Vector3 pos;
					pos.x = -physical_extent.x * 0.5f + ix * voxel_size.x;
					pos.y = -physical_extent.y * 0.5f + iy * voxel_size.y;
					pos.z = -physical_extent.z * 0.5f + iz * voxel_size.z;

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
	auto index_to_pos = [this](int x, int y, int z) -> Vector3 {
		int total_voxels_x = world_size.x * chunk_size;
		int total_voxels_y = world_size.y * chunk_size;
		int total_voxels_z = world_size.z * chunk_size;
		return Vector3(
				-total_voxels_x * 0.5f + x * voxel_size.x,
				-total_voxels_y * 0.5f + y * voxel_size.y,
				-total_voxels_z * 0.5f + z * voxel_size.z);
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

	return density_cache[density_cache_index(ix, iy, iz)];
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
	if (generation_in_progress.load()) {
		log_message("Generation already in progress, skipping regenerate_dirty_chunks", 1);
		return;
	}

	generation_in_progress.store(true);

	log_message("Regenerating dirty chunks...", 2);

	// For now, we do synchronous regeneration of dirty chunks
	// This can be extended to use WorkerThreadPool for async operation
	int regenerated_count = 0;

	for (int idx = 0; idx < static_cast<int>(chunk_dirty_flags.size()); ++idx) {
		bool is_dirty;
		{
			std::lock_guard<std::mutex> lock(chunks_mutex);
			is_dirty = chunk_dirty_flags[idx];
		}
		if (is_dirty) {
			Vector3i chunk_coord = index_to_chunk_coord(idx);

			// Regenerate this chunk's mesh
			// For now, just mark it clean - full per-chunk regeneration
			// would require refactoring generate() into per-chunk operations
			{
				std::lock_guard<std::mutex> lock(chunks_mutex);
				chunk_dirty_flags[idx] = false;
			}
			regenerated_count++;

			log_message(String("Regenerated chunk ({0},{1},{2})").format(Array::make(chunk_coord.x, chunk_coord.y, chunk_coord.z)), 3);
		}
	}

	log_message(String("Regenerated {0} dirty chunks").format(Array::make(regenerated_count)), 2);

	generation_in_progress.store(false);
}

void VoxelGenerator::invalidate_density_region(const Vector3i &min_voxel, const Vector3i &max_voxel) {
	log_message(String("Invalidating density region from ({0},{1},{2}) to ({3},{4},{5})").format(Array::make(min_voxel.x, min_voxel.y, min_voxel.z, max_voxel.x, max_voxel.y, max_voxel.z)), 2);

	// Calculate which chunks are affected by this region
	Vector3i min_chunk(
			min_voxel.x / chunk_size,
			min_voxel.y / chunk_size,
			min_voxel.z / chunk_size);
	Vector3i max_chunk(
			max_voxel.x / chunk_size,
			max_voxel.y / chunk_size,
			max_voxel.z / chunk_size);

	// Clamp to valid range
	min_chunk.x = std::max(0, min_chunk.x);
	min_chunk.y = std::max(0, min_chunk.y);
	min_chunk.z = std::max(0, min_chunk.z);
	max_chunk.x = std::min(world_size.x - 1, max_chunk.x);
	max_chunk.y = std::min(world_size.y - 1, max_chunk.y);
	max_chunk.z = std::min(world_size.z - 1, max_chunk.z);

	// Mark all affected chunks as dirty
	for (int cz = min_chunk.z; cz <= max_chunk.z; ++cz) {
		for (int cy = min_chunk.y; cy <= max_chunk.y; ++cy) {
			for (int cx = min_chunk.x; cx <= max_chunk.x; ++cx) {
				mark_chunk_dirty(Vector3i(cx, cy, cz));
			}
		}
	}

	// Optionally update the density cache for the affected region
	// For now, we clear the entire cache and rebuild on next generate()
	// A more sophisticated approach would update only the affected entries
	clear_density_cache();
}

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
	// Per-chunk mesh generation for async operation
	// This is a stub - full implementation would extract the per-chunk
	// portion of generate() into this method

	Vector3i chunk_coord = index_to_chunk_coord(chunk_index);
	if (chunk_coord.x < 0) {
		return; // Invalid index
	}

	log_message(String("Async generating chunk ({0},{1},{2})").format(Array::make(chunk_coord.x, chunk_coord.y, chunk_coord.z)), 3);

	// TODO: Implement per-chunk mesh generation
	// This would involve:
	// 1. Reading density values from cache (thread-safe via shared_lock)
	// 2. Running marching cubes for this chunk's voxels
	// 3. Building the mesh data (PackedVector3Array, etc.)
	// 4. Thread-safe storage of the result for later application

	// Mark chunk as clean after generation
	{
		std::lock_guard<std::mutex> lock(chunks_mutex);
		if (chunk_index >= 0 && chunk_index < static_cast<int>(chunk_dirty_flags.size())) {
			chunk_dirty_flags[chunk_index] = false;
		}
	}
}
} // namespace voxel_engine
