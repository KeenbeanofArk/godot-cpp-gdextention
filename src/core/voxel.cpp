#include "voxel.h"

// Godot includes
#include <godot_cpp/core/class_db.hpp>

namespace voxel_engine {
void Voxel::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_size"), &Voxel::get_size);
	ClassDB::bind_method(D_METHOD("set_size"), &Voxel::set_size);
	ClassDB::bind_method(D_METHOD("get_position"), &Voxel::get_position);
	ClassDB::bind_method(D_METHOD("set_position", "position"), &Voxel::set_position);
	ClassDB::bind_method(D_METHOD("get_type"), &Voxel::get_type);
	ClassDB::bind_method(D_METHOD("set_type", "type"), &Voxel::set_type);
	ClassDB::bind_method(D_METHOD("is_solid"), &Voxel::is_solid);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "position"), "set_position", "get_position");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "type"), "set_type", "get_type");

	// Bind enum constants properly
	BIND_ENUM_CONSTANT(AIR);
	BIND_ENUM_CONSTANT(DIRT);
	BIND_ENUM_CONSTANT(GRASS);
	BIND_ENUM_CONSTANT(STONE);
	BIND_ENUM_CONSTANT(WATER);
	BIND_ENUM_CONSTANT(SAND);
	BIND_ENUM_CONSTANT(LAVA);
	BIND_ENUM_CONSTANT(GOLD);
	BIND_ENUM_CONSTANT(DIAMOND);
	BIND_ENUM_CONSTANT(IRON);
	BIND_ENUM_CONSTANT(COAL);
}

Voxel::Voxel() {
	// Default constructor implementation
	size = 1; // Default size
	position = Vector3(0, 0, 0);
	type = VoxelType::DIRT;
}

Voxel::~Voxel() {
	// Destructor implementation
}

Vector3 Voxel::get_position() const {
	return position;
}

void Voxel::set_position(const Vector3 &p_position) {
	position = p_position;
}

int Voxel::get_size() const {
 return size;
}

void Voxel::set_size(int p_size) {
 if (p_size > 0) {
  size = p_size;
 } else {
  size = 1; // Default to 1 if invalid size is set
 }
}

int Voxel::get_type() const {
	return static_cast<int>(type);
}

void Voxel::set_type(int p_type) {
	type = static_cast<VoxelType>(p_type);
}

bool Voxel::is_solid() const {
	return type != static_cast<int>(VoxelType::AIR);
}

} // namespace voxel_engine