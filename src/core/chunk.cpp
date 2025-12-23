/**************************************************************************/
/*  chunk.cpp                                                             */
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

#include "chunk.h"
#include "Constants.h"
#include "voxel.h"
#include "voxel_constants.h"

// Godot includes
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/concave_polygon_shape3d.hpp>
#include <godot_cpp/classes/static_body3d.hpp>
#include <godot_cpp/classes/surface_tool.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>

namespace voxel_engine {

int voxel_engine::Chunk::default_chunk_size = Constants::DEFAULT_CHUNK_SIZE;

void Chunk::_bind_methods() {
	ClassDB::bind_method(D_METHOD("generate"), &Chunk::generate);
	// Bind the biome generator methods
	ClassDB::bind_method(D_METHOD("set_biome_generator", "generator"), &Chunk::set_biome_generator);
	ClassDB::bind_method(D_METHOD("get_biome_generator"), &Chunk::get_biome_generator);
	ClassDB::bind_method(D_METHOD("set_voxel", "local_pos", "type"), &Chunk::set_voxel);
	ClassDB::bind_method(D_METHOD("get_voxel", "local_pos"), &Chunk::get_voxel);
	ClassDB::bind_method(D_METHOD("get_chunk_size"), &Chunk::get_chunk_size);
	ClassDB::bind_method(D_METHOD("set_chunk_size", "chunk_size"), &Chunk::set_chunk_size);
	ClassDB::bind_method(D_METHOD("rebuild_mesh"), &Chunk::rebuild_mesh);
	ClassDB::bind_method(D_METHOD("update_lod", "camera_position"), &Chunk::update_lod);
	ClassDB::bind_method(D_METHOD("is_voxel_solid", "local_pos"), &Chunk::is_voxel_solid);
	ClassDB::bind_method(D_METHOD("notify_neighbor_chunks_if_on_border", "local_pos"), &Chunk::notify_neighbor_chunks_if_on_border);
	ClassDB::bind_method(D_METHOD("get_voxel_material_category_id", "local_pos"), &Chunk::get_voxel_material_category_id);

	// Chunk coordinate bindings (exposed for debugging)
	ClassDB::bind_method(D_METHOD("set_chunk_coord", "coord"), &Chunk::set_chunk_coord);
	ClassDB::bind_method(D_METHOD("get_chunk_coord"), &Chunk::get_chunk_coord);

	// Mesh methods
	ClassDB::bind_method(D_METHOD("apply_mesh_data", "vertices", "normals", "colors", "custom0"), &Chunk::apply_mesh_data);
	ClassDB::bind_method(D_METHOD("set_terrain_material", "material"), &Chunk::set_terrain_material);
	ClassDB::bind_method(D_METHOD("get_terrain_material"), &Chunk::get_terrain_material);
	ClassDB::bind_method(D_METHOD("clear_mesh"), &Chunk::clear_mesh);
	ClassDB::bind_method(D_METHOD("is_mesh_ready"), &Chunk::is_mesh_ready);

	// Properties
	ADD_GROUP("Chunk Info", "");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3I, "chunk_coord"), "set_chunk_coord", "get_chunk_coord");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "terrain_material", PROPERTY_HINT_RESOURCE_TYPE, "Material"), "set_terrain_material", "get_terrain_material");
}

void Chunk::set_terrain_material(const Ref<Material> &material) {
	terrain_material = material;
	if (mesh_instance) {
		mesh_instance->set_material_override(terrain_material);
	}
}

Ref<Material> Chunk::get_terrain_material() const {
	return terrain_material;
}

Chunk::Chunk() {
	m_chunk_size = default_chunk_size; // Use static default
	size_cubed = m_chunk_size * m_chunk_size * m_chunk_size;

	// Allocate flattened 1D array on heap using smart pointer
	voxels = std::make_unique<Ref<Voxel>[]>(size_cubed);

	position = Vector3();
	current_lod_level = 0;
	chunk_coord = Vector3i(0, 0, 0);
	mesh_instance = nullptr;
	mesh_ready.store(false);

	// Initialize all voxels to air
	for (int x = 0; x < m_chunk_size; ++x) {
		for (int y = 0; y < m_chunk_size; ++y) {
			for (int z = 0; z < m_chunk_size; ++z) {
				int idx = get_voxel_index(x, y, z);
				voxels[idx].instantiate();
				voxels[idx]->set_type(VoxelType::AIR);
				voxels[idx]->set_position(Vector3(x, y, z));
			}
		}
	}
}

Chunk::~Chunk() {
	// Clean up mesh instance if it exists
	if (mesh_instance && is_inside_tree()) {
		remove_child(mesh_instance);
		mesh_instance->queue_free();
		mesh_instance = nullptr;
	}
	if (collision_body && is_inside_tree()) {
		remove_child(collision_body);
		collision_body->queue_free();
		collision_body = nullptr;
		collision_shape = nullptr;
	}
	concave_shape.unref();
	// Smart pointer automatically cleans up voxel array
	// No manual delete needed
}

void Chunk::generate() {
	// Basic chunk generation - fill with dirt
	for (int x = 0; x < m_chunk_size; ++x) {
		for (int y = 0; y < m_chunk_size; ++y) {
			for (int z = 0; z < m_chunk_size; ++z) {
				int idx = get_voxel_index(x, y, z);
				if (y < m_chunk_size / 2) {
					voxels[idx]->set_type(VoxelType::DIRT);
				} else {
					voxels[idx]->set_type(VoxelType::AIR);
				}
			}
		}
	}
	// Rebuild the mesh after generation
	rebuild_mesh();
}

void Chunk::set_biome_generator(const Ref<voxel_engine::BiomeGenerator> &generator) {
	biome_generator = generator;
}

Ref<voxel_engine::BiomeGenerator> Chunk::get_biome_generator() const {
	return biome_generator;
}

void Chunk::set_chunk_size(int p_chunk_size) {
	if (p_chunk_size > 0 && p_chunk_size <= 64 && p_chunk_size != m_chunk_size) {
		m_chunk_size = p_chunk_size;
		size_cubed = m_chunk_size * m_chunk_size * m_chunk_size;

		// Reallocate voxel array with new size
		voxels = std::make_unique<Ref<Voxel>[]>(size_cubed);

		// Reinitialize all voxels to air
		for (int x = 0; x < m_chunk_size; ++x) {
			for (int y = 0; y < m_chunk_size; ++y) {
				for (int z = 0; z < m_chunk_size; ++z) {
					int idx = get_voxel_index(x, y, z);
					voxels[idx].instantiate();
					voxels[idx]->set_type(VoxelType::AIR);
					voxels[idx]->set_position(Vector3(x, y, z));
				}
			}
		}
	}
}

int Chunk::get_chunk_size() const {
	return m_chunk_size;
}

int Chunk::get_default_chunk_size() {
	return default_chunk_size;
}

void Chunk::set_default_chunk_size(int p_chunk_size) {
	if (p_chunk_size > 0 && p_chunk_size <= 64) {
		default_chunk_size = p_chunk_size;
	}
}

void Chunk::set_voxel(Vector3i local_pos, int type) {
	std::lock_guard<std::mutex> lock(voxel_mutex);
	if (local_pos.x >= 0 && local_pos.x < m_chunk_size &&
			local_pos.y >= 0 && local_pos.y < m_chunk_size &&
			local_pos.z >= 0 && local_pos.z < m_chunk_size) {
		int idx = get_voxel_index(local_pos.x, local_pos.y, local_pos.z);
		voxels[idx]->set_type(type);
	}
}

Ref<Voxel> Chunk::get_voxel(Vector3i local_pos) {
	std::lock_guard<std::mutex> lock(voxel_mutex);
	if (local_pos.x >= 0 && local_pos.x < m_chunk_size &&
			local_pos.y >= 0 && local_pos.y < m_chunk_size &&
			local_pos.z >= 0 && local_pos.z < m_chunk_size) {
		int idx = get_voxel_index(local_pos.x, local_pos.y, local_pos.z);
		return voxels[idx];
	}

	// Return empty voxel (air) if out of bounds
	Ref<Voxel> empty_voxel;
	empty_voxel.instantiate();
	empty_voxel->set_type(VoxelType::AIR);
	return empty_voxel;
}

void Chunk::rebuild_mesh() {
	// Basic mesh rebuilding - placeholder
	rebuild_mesh_with_lod(current_lod_level);
}

void Chunk::rebuild_mesh_with_lod(int lod_level) {
	// Placeholder for LOD mesh building
	current_lod_level = lod_level;
}

void Chunk::set_current_lod_level(int lod) {
	current_lod_level = lod;
}

int Chunk::get_current_lod_level() const {
	return current_lod_level;
}

void Chunk::update_lod(Vector3 camera_position) {
	float distance = position.distance_to(camera_position);
	int new_lod = 0;

	if (distance > 50.0f) {
		new_lod = 2;
	} else if (distance > 25.0f) {
		new_lod = 1;
	}

	if (new_lod != current_lod_level) {
		current_lod_level = new_lod;
		rebuild_mesh_with_lod(current_lod_level);
	}
}

bool Chunk::is_voxel_solid(Vector3i local_pos) {
	std::lock_guard<std::mutex> lock(voxel_mutex);
	if (local_pos.x >= 0 && local_pos.x < m_chunk_size &&
			local_pos.y >= 0 && local_pos.y < m_chunk_size &&
			local_pos.z >= 0 && local_pos.z < m_chunk_size) {
		int idx = get_voxel_index(local_pos.x, local_pos.y, local_pos.z);
		return voxels[idx]->is_solid();
	}
	return false;
}

void Chunk::notify_neighbor_chunks_if_on_border(Vector3i local_pos) {
	// Placeholder for neighbor notification
	// This would typically notify adjacent chunks when voxels on the border change
}

int Chunk::get_voxel_material_category_id(Vector3i local_pos) {
	std::lock_guard<std::mutex> lock(voxel_mutex);
	if (local_pos.x >= 0 && local_pos.x < m_chunk_size &&
			local_pos.y >= 0 && local_pos.y < m_chunk_size &&
			local_pos.z >= 0 && local_pos.z < m_chunk_size) {
		int idx = get_voxel_index(local_pos.x, local_pos.y, local_pos.z);
		return voxels[idx]->get_type();
	}
	return VoxelType::AIR;
}

// ============================================================================
// Chunk Coordinate Methods
// ============================================================================

void Chunk::set_chunk_coord(const Vector3i &coord) {
	chunk_coord = coord;
}

Vector3i Chunk::get_chunk_coord() const {
	return chunk_coord;
}

// ============================================================================
// Mesh Application Methods (Main Thread Only)
// ============================================================================

void Chunk::apply_mesh_data(const PackedVector3Array &vertices, const PackedVector3Array &normals, const PackedColorArray &colors, const PackedColorArray &custom0) {
	// This must be called from the main thread only!

	// Clear existing mesh if any
	clear_mesh();

	// Don't create mesh if no vertices
	if (vertices.size() == 0) {
		mesh_ready.store(true);
		return;
	}

	// Create ArrayMesh from the data
	Ref<ArrayMesh> array_mesh;
	array_mesh.instantiate();

	Array arrays;
	arrays.resize(Mesh::ARRAY_MAX);
	arrays[Mesh::ARRAY_VERTEX] = vertices;
	arrays[Mesh::ARRAY_NORMAL] = normals;
	arrays[Mesh::ARRAY_COLOR] = colors;

	// CUSTOM0 is uploaded as RGBA8_UNORM bytes for compatibility with Godot's custom attribute formats.
	PackedByteArray custom0_bytes;
	int vertex_count = vertices.size();
	custom0_bytes.resize(vertex_count * 4);
	for (int i = 0; i < vertex_count; ++i) {
		Color c = (i < custom0.size()) ? custom0[i] : Color(0, 0, 0, 0);
		int base = i * 4;
		custom0_bytes[base + 0] = static_cast<uint8_t>(CLAMP(int(Math::round(c.r * 255.0f)), 0, 255));
		custom0_bytes[base + 1] = static_cast<uint8_t>(CLAMP(int(Math::round(c.g * 255.0f)), 0, 255));
		custom0_bytes[base + 2] = static_cast<uint8_t>(CLAMP(int(Math::round(c.b * 255.0f)), 0, 255));
		custom0_bytes[base + 3] = static_cast<uint8_t>(CLAMP(int(Math::round(c.a * 255.0f)), 0, 255));
	}
	arrays[Mesh::ARRAY_CUSTOM0] = custom0_bytes;

	uint64_t format = Mesh::ARRAY_FORMAT_VERTEX | Mesh::ARRAY_FORMAT_NORMAL | Mesh::ARRAY_FORMAT_COLOR | Mesh::ARRAY_FORMAT_CUSTOM0;
	format |= (uint64_t)ArrayMesh::ARRAY_CUSTOM_RGBA8_UNORM << Mesh::ARRAY_FORMAT_CUSTOM0_SHIFT;

	array_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays, Array(), Dictionary(), format);

	// Default material (used only when no override material is assigned).
	if (terrain_material.is_null()) {
		Ref<StandardMaterial3D> material;
		material.instantiate();
		material->set_flag(BaseMaterial3D::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
		array_mesh->surface_set_material(0, material);
	}

	// Create MeshInstance3D if needed
	if (!mesh_instance) {
		mesh_instance = memnew(MeshInstance3D);
		mesh_instance->set_name("ChunkMesh");
		add_child(mesh_instance);
	}

	mesh_instance->set_mesh(array_mesh);
	mesh_instance->set_material_override(terrain_material);
	update_collision_shape(array_mesh);
	mesh_ready.store(true);
}

void Chunk::clear_mesh() {
	if (mesh_instance) {
		mesh_instance->set_mesh(Ref<Mesh>());
	}
	clear_collision_shape();
	mesh_ready.store(false);
}

bool Chunk::is_mesh_ready() const {
	return mesh_ready.load();
}

void Chunk::clear_collision_shape() {
	if (collision_shape) {
		collision_shape->set_shape(Ref<Shape3D>());
	}
	concave_shape.unref();
}

void Chunk::update_collision_shape(const Ref<ArrayMesh> &mesh) {
	if (mesh.is_null() || mesh->get_surface_count() == 0) {
		clear_collision_shape();
		return;
	}

	PackedVector3Array faces;
	for (int surface = 0; surface < mesh->get_surface_count(); ++surface) {
		Array arrays = mesh->surface_get_arrays(surface);
		PackedVector3Array surface_vertices = arrays[Mesh::ARRAY_VERTEX];
		if (surface_vertices.is_empty()) {
			continue;
		}
		faces.append_array(surface_vertices);
	}

	if (faces.size() < 3) {
		clear_collision_shape();
		return;
	}

	if (!collision_body) {
		collision_body = memnew(StaticBody3D);
		collision_body->set_name("ChunkCollider");
		collision_body->set_collision_layer(1); // World layer (bit 0)
		collision_body->set_collision_mask(2 | 4); // Collide with players (bit 1) and creatures (bit 2)
		add_child(collision_body);

		// Ensure the physics body is centered relative to the chunk's mesh (world-space meshes use chunk as origin)
		collision_body->set_position(Vector3(0, 0, 0));
	}

	if (!collision_shape) {
		collision_shape = memnew(CollisionShape3D);
		collision_shape->set_name("ChunkCollisionShape");
		collision_body->add_child(collision_shape);

		// Center the collision shape locally inside the StaticBody3D
		collision_shape->set_position(Vector3(0, 0, 0));
	}

	if (concave_shape.is_null()) {
		concave_shape.instantiate();
	}

	concave_shape->set_faces(faces);
	collision_shape->set_shape(concave_shape);
}

} // namespace voxel_engine