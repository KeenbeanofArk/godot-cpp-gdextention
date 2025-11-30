/**************************************************************************/
/*  VoxelGenerator.h                                                      */
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

#ifndef VOXEL_GENERATOR_H
#define VOXEL_GENERATOR_H

// We don't need windows.h in this example plugin but many others do, and it can
// lead to annoying situations due to the ton of macros it defines.
// So we include it and make sure CI warns us if we use something that conflicts
// with a Windows define.
#ifdef WIN32
#include <windows.h>
#endif

#include "Constants.h"
#include "core/chunk.h"
#include "core/voxel.h"
#include "generators/NoiseGenerator.h"

// Threading support
#include <atomic>
#include <mutex>
#include <shared_mutex>

#include <godot_cpp/classes/fast_noise_lite.hpp>
#include <godot_cpp/classes/immediate_mesh.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/worker_thread_pool.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/vector3.hpp>

using namespace godot;

namespace voxel_engine {

class VoxelGenerator : public Node3D {
	GDCLASS(VoxelGenerator, Node3D)

private:
	// Terrain generation properties
	Ref<NoiseGenerator> terrain_noise; // 2D noise for base terrain height
	Ref<NoiseGenerator> detail_noise; // 3D noise for rocky detail
	float rock_influence = 0.3f; // Strength of rocky detail (0.0-1.0)
	float terrain_height = 4.0f; // Base terrain height in world units
	float terrain_amplitude = 8.0f; // Height variation range

	inline static Vector3i world_size = Vector3i(1, 1, 1);
	int resolution = 1;
	float cutoff = 0.0f;
	bool show_centers = false;
	bool show_voxel_grid = false;
	bool show_chunk_grid = false;
	int seeder = 1240;
	bool auto_generate = true;
	bool vertex_limit = false; // Limit the number of vertices generated

	// Debug properties
	bool debug_mode = true;
	bool visualize_noise_values = false;
	int debug_verbosity = 1;

	// Add a container for chunks, e.g.:
	std::vector<Chunk *> chunks;
	int chunk_size = Constants::DEFAULT_CHUNK_SIZE; // Default value from voxel_constants.h

	// ==================== Threading & Density Cache ====================
	// Density cache for marching cubes optimization (stores corner densities)
	std::vector<float> density_cache;
	int cache_size_x = 0; // Number of corner points in X (total_voxels_x + 1)
	int cache_size_y = 0; // Number of corner points in Y (total_voxels_y + 1)
	int cache_size_z = 0; // Number of corner points in Z (total_voxels_z + 1)

	// Thread synchronization primitives
	mutable std::shared_mutex density_cache_mutex; // Read-write lock for density cache
	mutable std::mutex chunks_mutex; // Protects chunks vector modifications
	std::atomic<bool> generation_in_progress{ false }; // Prevents concurrent generate() calls

	// Chunk dirty flags for incremental updates (protected by chunks_mutex)
	std::vector<bool> chunk_dirty_flags;
	// ===================================================================

	const bool object_instance_binding_set_by_parent_constructor;
	bool has_object_instance_binding() const;

protected:
	static void _bind_methods();
	// This method is called to notify the object of various events.
	void _notification(int p_what);

public:
	VoxelGenerator();
	~VoxelGenerator();

	void set_world_size(const Vector3i &value);
	Vector3i get_world_size() const;

	// Terrain noise getters/setters
	void set_terrain_noise(const Ref<NoiseGenerator> &p_noise);
	Ref<NoiseGenerator> get_terrain_noise() const;

	void set_detail_noise(const Ref<NoiseGenerator> &p_noise);
	Ref<NoiseGenerator> get_detail_noise() const;

	void set_rock_influence(float value);
	float get_rock_influence() const;

	void set_terrain_height(float value);
	float get_terrain_height() const;

	void set_terrain_amplitude(float value);
	float get_terrain_amplitude() const;

	void set_resolution(int value);
	int get_resolution() const;

	void set_cutoff(float value);
	float get_cutoff() const;

	void set_randomizer(bool value);
	bool get_randomizer() const;

	void set_show_centers(bool value);
	bool get_show_centers() const;

	void set_show_voxel_grid(bool value);
	bool get_show_voxel_grid() const;

	void set_show_chunk_grid(bool value);
	bool get_show_chunk_grid() const;

	void set_seeder(int value);
	int get_seeder() const;

	void set_auto_generate(bool value);
	bool get_auto_generate() const;

	void set_vertex_limit(bool value);
	bool get_vertex_limit() const;

	void set_chunk_size(int value);
	int get_chunk_size() const;

	void reset();

	void generate();

	// Debug methods
	void set_debug_mode(bool p_enabled);
	bool get_debug_mode() const;

	void set_visualize_noise_values(bool p_enabled);
	bool get_visualize_noise_values() const;

	void set_debug_verbosity(int p_level);
	int get_debug_verbosity() const;

	void debug_print_state();
	void debug_draw_noise_slice(float y_level);
	void log_message(const String &message, int verbosity_level = 1);

	bool is_object_binding_set_by_parent_constructor() const;

	// ==================== Chunk Dirty Flag API ====================
	// Mark a specific chunk as needing regeneration
	void mark_chunk_dirty(const Vector3i &chunk_coord);
	// Mark all chunks as dirty
	void mark_all_chunks_dirty();
	// Check if a chunk is dirty
	bool is_chunk_dirty(const Vector3i &chunk_coord) const;
	// Regenerate only dirty chunks (incremental update)
	void regenerate_dirty_chunks();
	// Invalidate density cache for a region (call when terrain is edited)
	void invalidate_density_region(const Vector3i &min_voxel, const Vector3i &max_voxel);
	// ==============================================================

private:
	void remove_children();
	void randomize_seed();
	Vector<Vector3> create_cube_vertices(const Vector3 &pos);
	void add_cubes_vertices(Ref<ImmediateMesh> mesh, const Vector<Vector3> &vertices);
	float get_terrain_density(const Vector3 &pos) const;
	std::vector<float> get_cube_values(const Vector<Vector3> &cube_vertices);
	int get_lookup_index(const std::vector<float> &cube_values, float cutoff);
	Vector3 interpolate(const Vector3 &vertex_1, float value_1, const Vector3 &vertex_2, float value_2);
	void add_cube_edges(Ref<ImmediateMesh> mesh, const std::vector<Vector3> &v);
	void calculate_world_size();

	// Debug helpers
	void create_debug_visualization();
	void visualize_noise_field();

	// Optionally, add helpers to manage chunks/voxels
	void create_chunks();
	void fill_chunk_with_voxels(Chunk *chunk);

	// Derived voxel scale (physical size per sample). For Option A: resolution = samples per voxel edge
	Vector3 voxel_size = Vector3(1, 1, 1);

	// Recalculate voxel_size when world/chunk/resolution parameters change
	void recalculate_voxel_scale();

	bool is_instance_valid(Chunk *chunk) const;

	// ==================== Density Cache Methods ====================
	// Build the density cache for all corner positions (call at start of generate())
	void build_density_cache();
	// Clear the density cache to free memory (call at end of generate() or on reset)
	void clear_density_cache();
	// Get cached density value (thread-safe read)
	float get_cached_density(int ix, int iy, int iz) const;
	// Convert 3D index to 1D cache index
	inline int density_cache_index(int ix, int iy, int iz) const {
		return ix + iy * cache_size_x + iz * cache_size_x * cache_size_y;
	}
	// Get cube corner values from cache (optimized version)
	std::vector<float> get_cube_values_cached(int ix, int iy, int iz) const;
	// Convert chunk coordinate to linear index
	int chunk_coord_to_index(const Vector3i &chunk_coord) const;
	// Convert linear index to chunk coordinate
	Vector3i index_to_chunk_coord(int index) const;
	// ==============================================================

	// ==================== Async Generation ====================
	// Static callback for WorkerThreadPool
	static void _chunk_generation_task(void *userdata, uint32_t chunk_index);
	// Internal chunk mesh generation (called from worker thread)
	void generate_chunk_mesh_internal(int chunk_index);
	// ===========================================================
};
} // namespace voxel_engine

#endif // VOXEL_GENERATOR_H