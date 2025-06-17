// voxel.h

#ifndef VOXEL_H
#define VOXEL_H

// Godot includes
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/vector3.hpp>

using namespace godot;

namespace voxel_engine {

// Move the enum outside the class to make it accessible to Godot's type system
enum VoxelType {
	AIR,
	DIRT,
	GRASS,
	STONE,
	WATER,
	SAND,
	LAVA,
	GOLD,
	DIAMOND,
	IRON,
	COAL
	// Add more voxel types as needed
};

class Voxel : public RefCounted {
	GDCLASS(Voxel, RefCounted);

protected:
	static void _bind_methods();

public:
	Voxel();
	~Voxel();

	Vector3 get_position() const;
	void set_position(const Vector3 &position);

	int get_type() const;
	void set_type(int type);

	bool is_solid() const;

	void set_size(int size);
	int get_size() const;

private:
	Vector3 position;
	int type;
	int size;
};

} //namespace voxel_engine

// Register the VoxelType enum with Godot's type system
VARIANT_ENUM_CAST(voxel_engine::VoxelType);

#endif // VOXEL_H