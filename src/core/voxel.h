// voxel.h

#pragma once

// Godot includes
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

namespace voxel_engine
{

    class Voxel : public RefCounted
    {
        GDCLASS(Voxel, RefCounted);

    protected:
        static void _bind_methods();

    public:
        enum VoxelType
        {
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

        Voxel();
        ~Voxel();

        Vector3 get_position() const;
        void set_position(const Vector3 &position);

        int get_type() const;
        void set_type(int type);

        bool is_solid() const;

    private:
        Vector3 position;
        int type;
    };

} // VOXEL_H