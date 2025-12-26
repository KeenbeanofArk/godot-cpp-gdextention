/**************************************************************************/
/*  Chunk.h                                                               */
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

#ifndef CHUNK_H
#define CHUNK_H

#include <atomic>
#include <memory>
#include <mutex>

#include "direction.h"
#include "generators/BiomeGenerator.h"
#include "voxel.h"
#include "voxel_constants.h"

// Godot includes
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/concave_polygon_shape3d.hpp>
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/static_body3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/packed_color_array.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/vector3i.hpp>

using namespace godot;

namespace voxel_engine {

class Chunk : public Node3D {
	GDCLASS(Chunk, Node3D);

protected:
	static void _bind_methods();

public:
	int chunk_id = 0; // Unique identifier for the chunk
	Vector3i chunk_coord{ 0, 0, 0 }; // Grid position in chunk coordinates
	std::unique_ptr<Ref<Voxel>[]> voxels; // Flattened 1D array for cache efficiency (smart pointer)
	Vector3 position;

	// Mesh ownership - chunk owns its own mesh instance
	MeshInstance3D *mesh_instance = nullptr;
	std::atomic<bool> mesh_ready{ false };
	StaticBody3D *collision_body = nullptr;
	CollisionShape3D *collision_shape = nullptr;
	Ref<ConcavePolygonShape3D> concave_shape;

	// Thread safety for voxel access
	mutable std::mutex voxel_mutex;

	Chunk();
	~Chunk();

	void generate();

	void set_biome_generator(const Ref<voxel_engine::BiomeGenerator> &generator);
	Ref<voxel_engine::BiomeGenerator> get_biome_generator() const;

	void set_voxel(Vector3i local_pos, int type);
	Ref<Voxel> get_voxel(Vector3i local_pos);

	// Chunk coordinate accessors (exposed to GDScript for debugging)
	void set_chunk_coord(const Vector3i &coord);
	Vector3i get_chunk_coord() const;

	// Mesh data application (called from main thread only)
	void apply_mesh_data(const PackedVector3Array &vertices, const PackedVector3Array &normals, const PackedColorArray &colors, const PackedColorArray &custom0);
	void set_terrain_material(const Ref<Material> &material);
	Ref<Material> get_terrain_material() const;
	void clear_mesh();
	bool is_mesh_ready() const;
	void update_collision_shape(const Ref<ArrayMesh> &mesh);
	void clear_collision_shape();

	// Helper to apply arrays (used by loader to set mesh data on main thread)
	void set_mesh_from_arrays(const PackedVector3Array &vertices, const PackedVector3Array &normals, const PackedColorArray &colors, const PackedColorArray &custom0) {
		apply_mesh_data(vertices, normals, colors, custom0);
	}

	void set_chunk_size(int p_chunk_size);
	int get_chunk_size() const;
	static int get_default_chunk_size();
	static void set_default_chunk_size(int p_chunk_size);

	void rebuild_mesh();
	void update_lod(Vector3 camera_position);
	bool is_voxel_solid(Vector3i local_pos);
	void notify_neighbor_chunks_if_on_border(Vector3i local_pos);
	int get_voxel_material_category_id(Vector3i local_pos);

	// LOD level accessor methods
	void set_current_lod_level(int lod);
	int get_current_lod_level() const;

private:
	Ref<voxel_engine::BiomeGenerator> biome_generator;
	Ref<Material> terrain_material;

	// Static default chunk size (class-wide default)
	static int default_chunk_size;
	// Instance chunk size (actual size for this chunk)
	int m_chunk_size = 0;
	int size_cubed = 0; // Cached size^3 for voxel array

	// Inline accessor for 1D flattened array indexing
	inline int get_voxel_index(int x, int y, int z) const {
		return x * m_chunk_size * m_chunk_size + y * m_chunk_size + z;
	}

	// Private helper methods can be added here if needed
	int current_lod_level = 0; // Current LOD level
	void rebuild_mesh_with_lod(int lod_level);
};

} // namespace voxel_engine

#endif // CHUNK_H