#include "voxel.h"

// Godot includes
#include <godot_cpp/core/class_db.hpp>

namespace voxel_engine
{
    void Voxel::_bind_methods()
    {
        ClassDB::bind_method(D_METHOD("get_position"), &Voxel::get_position);
        ClassDB::bind_method(D_METHOD("set_position", "position"), &Voxel::set_position);
        ClassDB::bind_method(D_METHOD("get_type"), &Voxel::get_type);
        ClassDB::bind_method(D_METHOD("set_type", "type"), &Voxel::set_type);
        ClassDB::bind_method(D_METHOD("is_solid"), &Voxel::is_solid);
    }

    Voxel::Voxel()
    {
        // Default constructor implementation
        position = Vector3(0, 0, 0);
        type = VoxelType::AIR;
    }

    Voxel::~Voxel()
    {
        // Destructor implementation
    }

    Vector3 Voxel::get_position() const
    {
        return position;
    }

    void Voxel::set_position(const Vector3 &p_position)
    {
        position = p_position;
    }

    Voxel::VoxelType Voxel::get_type() const
    {
        return type;
    }

    void Voxel::set_type(VoxelType p_type)
    {
        type = p_type;
    }

    bool Voxel::is_solid() const
    {
        return type != VoxelType::AIR;
    }

} // namespace voxel_engine