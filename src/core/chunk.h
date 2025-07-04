// chunk.h

#ifndef CHUNK_H
#define CHUNK_H

#include "direction.h"
#include "voxel.h"

// Godot includes
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/vector3i.hpp>

using namespace godot;

namespace voxel_engine {

class Chunk : public Node3D {
	GDCLASS(Chunk, Node3D);

protected:
	static void _bind_methods();

public:
	int chunk_size = 8;
	inline static const Vector3i WORLD_SIZE = Vector3i(0, 0, 0);

	int chunk_id = 0; // Unique identifier for the chunk
	Ref<Voxel> voxels[8][8][8];
	Vector3 position;

	Chunk();
	~Chunk();

	void generate();
	void set_voxel(Vector3i local_pos, int type);
	Ref<Voxel> get_voxel(Vector3i local_pos);
	void set_chunk_size(int p_chunk_size);
	int get_chunk_size() const;
	void rebuild_mesh();
	void update_lod(Vector3 camera_position);
	bool is_voxel_solid(Vector3i local_pos);
	void notify_neighbor_chunks_if_on_border(Vector3i local_pos);
	int get_voxel_material_category_id(Vector3i local_pos);

private:
	// Private helper methods can be added here if needed
	int current_lod_level = 0; // Current LOD level
	void rebuild_mesh_with_lod(int lod_level);

private:
	//BiomeGenerator *biome_generator = nullptr;
};

} // namespace voxel_engine

#endif // CHUNK_H