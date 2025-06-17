#include "chunk.h"
#include "voxel.h"
#include "voxel_constants.h"

// Godot includes
#include <godot_cpp/core/class_db.hpp>

namespace voxel_engine {

void Chunk::_bind_methods() {
	ClassDB::bind_method(D_METHOD("generate"), &Chunk::generate);
	ClassDB::bind_method(D_METHOD("set_voxel", "local_pos", "type"), &Chunk::set_voxel);
	ClassDB::bind_method(D_METHOD("get_voxel", "local_pos"), &Chunk::get_voxel);
	ClassDB::bind_method(D_METHOD("set_chunk_size", "lod_level"), &Chunk::set_chunk_size);
	ClassDB::bind_method(D_METHOD("get_chunk_size"), &Chunk::get_chunk_size);
	ClassDB::bind_method(D_METHOD("rebuild_mesh"), &Chunk::rebuild_mesh);
	ClassDB::bind_method(D_METHOD("update_lod", "camera_position"), &Chunk::update_lod);
	ClassDB::bind_method(D_METHOD("is_voxel_solid", "local_pos"), &Chunk::is_voxel_solid);
	ClassDB::bind_method(D_METHOD("notify_neighbor_chunks_if_on_border", "local_pos"), &Chunk::notify_neighbor_chunks_if_on_border);
	ClassDB::bind_method(D_METHOD("get_voxel_material_category_id", "local_pos"), &Chunk::get_voxel_material_category_id);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "chunk_size", PROPERTY_HINT_RANGE, "8,64,8"), "set_chunk_size", "get_chunk_size");

}

Chunk::Chunk() {
	position = Vector3();
	current_lod_level = 0;
	// Initialize all voxels to air
	for (int x = 0; x < chunk_size; ++x) {
		for (int y = 0; y < chunk_size; ++y) {
			for (int z = 0; z < chunk_size; ++z) {
				voxels[x][y][z].instantiate();
				voxels[x][y][z]->set_type(VoxelType::AIR);
				voxels[x][y][z]->set_position(Vector3(x, y, z));
				UtilityFunctions::print("Voxel initialized at position: " + String::num_int64(x) + ", " + String::num_int64(y) + ", " + String::num_int64(z) +
							" with type: " + String::num_int64(voxels[x][y][z]->get_type()));
			}
		}
	}
}

Chunk::~Chunk() {
}

void Chunk::generate() {
	// Basic chunk generation - fill with dirt
	for (int x = 0; x < chunk_size; ++x) {
		for (int y = 0; y < chunk_size; ++y) {
			for (int z = 0; z < chunk_size; ++z) {
				if (y < chunk_size / 2) {
					voxels[x][y][z]->set_type(VoxelType::DIRT);
				} else {
					voxels[x][y][z]->set_type(VoxelType::AIR);
				}
				UtilityFunctions::print("Voxel at position: " + String::num_int64(x) + ", " + String::num_int64(y) + ", " + String::num_int64(z) +
	   " set to type: " + String::num_int64(voxels[x][y][z]->get_type()));
			}
		}
	}
	 // Rebuild the mesh after generation
	rebuild_mesh();
}

void Chunk::set_chunk_size(int p_chunk_size) {
	if (p_chunk_size > 0 && p_chunk_size <= 64) {
		chunk_size = p_chunk_size;
	}
}

int Chunk::get_chunk_size() const {
	return chunk_size;
}

void Chunk::set_voxel(Vector3i local_pos, int type) {
	if (local_pos.x >= 0 && local_pos.x < chunk_size &&
			local_pos.y >= 0 && local_pos.y < chunk_size &&
			local_pos.z >= 0 && local_pos.z < chunk_size) {
		voxels[local_pos.x][local_pos.y][local_pos.z]->set_type(type);
	}
}

Ref<Voxel> Chunk::get_voxel(Vector3i local_pos) {
	if (local_pos.x >= 0 && local_pos.x < chunk_size &&
			local_pos.y >= 0 && local_pos.y < chunk_size &&
			local_pos.z >= 0 && local_pos.z < chunk_size) {
		return voxels[local_pos.x][local_pos.y][local_pos.z];
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
	if (local_pos.x >= 0 && local_pos.x < chunk_size &&
			local_pos.y >= 0 && local_pos.y < chunk_size &&
			local_pos.z >= 0 && local_pos.z < chunk_size) {
		return voxels[local_pos.x][local_pos.y][local_pos.z]->is_solid();
	}
	return false;
}

void Chunk::notify_neighbor_chunks_if_on_border(Vector3i local_pos) {
	// Placeholder for neighbor notification
	// This would typically notify adjacent chunks when voxels on the border change
}

int Chunk::get_voxel_material_category_id(Vector3i local_pos) {
	if (local_pos.x >= 0 && local_pos.x < chunk_size &&
			local_pos.y >= 0 && local_pos.y < chunk_size &&
			local_pos.z >= 0 && local_pos.z < chunk_size) {
		return voxels[local_pos.x][local_pos.y][local_pos.z]->get_type();
	}
	return VoxelType::AIR;
}

} // namespace voxel_engine