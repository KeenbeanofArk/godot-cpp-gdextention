#include "chunk.h"
#include "voxel_constants.h"

// Godot includes
#include <godot_cpp/core/class_db.hpp>

namespace voxel_engine
{

    void Chunk::_bind_methods()
    {
        ClassDB::bind_method(D_METHOD("generate"), &Chunk::generate);
        ClassDB::bind_method(D_METHOD("set_voxel", "local_pos", "type"), &Chunk::set_voxel);
        ClassDB::bind_method(D_METHOD("get_voxel", "local_pos"), &Chunk::get_voxel);
        ClassDB::bind_method(D_METHOD("rebuild_mesh"), &Chunk::rebuild_mesh);
        ClassDB::bind_method(D_METHOD("update_lod", "camera_position"), &Chunk::update_lod);
        ClassDB::bind_method(D_METHOD("is_voxel_solid", "local_pos"), &Chunk::is_voxel_solid);
    }

    Chunk::Chunk()
    {
        // Initialize the first chunk with default values
        position = Vector3(0, 0, 0);

        // Note: Since we can't directly initialize the voxel array,
        // we'll initialize the voxels when needed in the get/set methods.
        // Alternatively, your implementation could use a dynamic structure
        // or initialize them in the generate() method.
    }

    Chunk::~Chunk()
    {
        // Destructor implementation
    }

    void Chunk::generate()
    {
        //if (!biome_generator)
            //return; // Safety check
        for (int x = 0; x < CHUNK_SIZE; ++x)
        {
            for (int y = 0; y < CHUNK_SIZE; ++y)
            {
                for (int z = 0; z < CHUNK_SIZE; ++z)
                {
                    Vector3i local_pos(x, y, z);
                    Vector3 world_pos = position * CHUNK_SIZE + Vector3(x, y, z);

                    // Use the biome generator instance to determine voxel type
                    //Ref<Voxel> voxel = biome_generator->get_voxel_at((int)world_pos.x, (int)world_pos.y, (int)world_pos.z);
                    //Voxel::VoxelType type = voxel.is_valid() ? voxel->get_type() : Voxel::AIR;

                    //voxels[x][y][z].set_type(type);
                    //voxels[x][y][z].set_position(world_pos);
                }
            }
        }
    }

    void Chunk::set_voxel(Vector3i local_pos, int type)
    {
        if (local_pos.x < 0 || local_pos.x >= CHUNK_SIZE ||
            local_pos.y < 0 || local_pos.y >= CHUNK_SIZE ||
            local_pos.z < 0 || local_pos.z >= CHUNK_SIZE)
        {
            return;
        }

        // Set the voxel type
        voxels[local_pos.x][local_pos.y][local_pos.z].set_type(type);

        // Optional: update position if needed
        Vector3 world_pos(
            position.x * CHUNK_SIZE + local_pos.x,
            position.y * CHUNK_SIZE + local_pos.y,
            position.z * CHUNK_SIZE + local_pos.z);

        voxels[local_pos.x][local_pos.y][local_pos.z].set_position(world_pos);

        // Update mesh and notify neighbors if needed
        rebuild_mesh();
        notify_neighbor_chunks_if_on_border(local_pos);
        UtilityFunctions::print("Set voxel at ", local_pos, " to type ", type);
    }

    Ref<Voxel> Chunk::get_voxel(Vector3i local_pos)
    {
        if (local_pos.x < 0 || local_pos.x >= CHUNK_SIZE ||
            local_pos.y < 0 || local_pos.y >= CHUNK_SIZE ||
            local_pos.z < 0 || local_pos.z >= CHUNK_SIZE)
        {
            // Return a default voxel if out of bounds
            Ref<Voxel> default_voxel;
            default_voxel.instantiate();
            return default_voxel;
        }

        // Create a new Voxel object and copy the data
        Ref<Voxel> result;
        result.instantiate();
        result->set_type(voxels[local_pos.x][local_pos.y][local_pos.z].get_type());
        result->set_position(voxels[local_pos.x][local_pos.y][local_pos.z].get_position());

        return result;
    }

    void Chunk::rebuild_mesh()
    {
        // Rebuild the mesh with the highest detail level (LOD 0)
        rebuild_mesh_with_lod(0);
    }

    void Chunk::update_lod(Vector3 camera_position)
    {
        // Calculate the distance from the chunk center to the camera
        Vector3 chunk_center = position * CHUNK_SIZE + Vector3(CHUNK_SIZE / 2, CHUNK_SIZE / 2, CHUNK_SIZE / 2);
        float distance = camera_position.distance_to(chunk_center);

        // Get LOD settings from voxel_engine
        // In practice, you'd have these passed in from the VoxelEngine class
        // or stored as a member variable in the Chunk class

        // Default LOD thresholds based on voxel_constants.h
        const int lod_levels = MAX_LOD_LEVELS;
        float lod_distances[MAX_LOD_LEVELS];

        // Initialize default LOD distances with multiplier
        for (int i = 0; i < lod_levels; i++)
        {
            // Each level is DEFAULT_LOD_DISTANCE_MULTIPLIER times further than the previous
            lod_distances[i] = CHUNK_SIZE * pow(DEFAULT_LOD_DISTANCE_MULTIPLIER, i + 1);
        }

        // Determine the appropriate LOD level based on distance
        int current_lod_level = lod_levels - 1; // Start with maximum (lowest detail)

        // Find the appropriate LOD level based on distance
        for (int i = 0; i < lod_levels; i++)
        {
            if (distance < lod_distances[i])
            {
                current_lod_level = i;
                break;
            }
        }

        // Store the current LOD level as a member variable or call rebuild_mesh with the LOD level
        // This depends on your implementation approach

        // Option 1: Store LOD level and rebuild the mesh if it changed
        static int previous_lod_level = -1; // Initialize to an invalid value to force first update
        if (previous_lod_level != current_lod_level)
        {
            previous_lod_level = current_lod_level;

            // Rebuild the mesh with the new LOD level
            // In a real implementation, you might pass this to the mesh builder
            rebuild_mesh_with_lod(current_lod_level);
        }

        // Option 2: Just rebuild the mesh with the determined LOD level every time
        // This is simpler but less efficient
        // rebuild_mesh_with_lod(current_lod_level);
    }

    // Helper method to rebuild mesh with specific LOD level
    // You would need to add this method to the Chunk class
    void Chunk::rebuild_mesh_with_lod(int lod_level)
    {
        // LOD level 0 is highest detail, higher numbers mean lower detail

        // Adjust voxel sampling based on LOD
        // For low detail (high LOD levels), we might skip voxels or use simplified mesh generation

        int step = 1 << lod_level; // 2^lod_level: 1, 2, 4, 8, etc.
        if (step > CHUNK_SIZE / 2)
        {
            step = CHUNK_SIZE / 2; // Cap the step size to avoid empty chunks
        }

        // Here you would use your mesh builder to create an appropriate mesh
        // with the given LOD level. For example:

        // Clear existing mesh data

        // For each voxel (with appropriate step size based on LOD level)
        for (int x = 0; x < CHUNK_SIZE; x += step)
        {
            for (int y = 0; y < CHUNK_SIZE; y += step)
            {
                for (int z = 0; z < CHUNK_SIZE; z += step)
                {
                    // For lower detail, you might want to represent multiple voxels as one
                    Vector3i local_pos(x, y, z);

                    // Only process if the voxel is solid
                    if (is_voxel_solid(local_pos))
                    {
                        // Add faces for this voxel to the mesh
                        // The actual mesh building would be more complex
                        // and might involve greedy meshing or other techniques

                        // For simplicity, we're just illustrating the concept here
                        // add_voxel_to_mesh(local_pos, step);
                    }
                }
            }
        }

        // Apply the mesh data to your mesh instance
    }

    bool Chunk::is_voxel_solid(Vector3i local_pos)
    {
        if (local_pos.x < 0 || local_pos.x >= CHUNK_SIZE ||
            local_pos.y < 0 || local_pos.y >= CHUNK_SIZE ||
            local_pos.z < 0 || local_pos.z >= CHUNK_SIZE)
        {
            return false;
        }

        return voxels[local_pos.x][local_pos.y][local_pos.z].is_solid();
    }

    void Chunk::notify_neighbor_chunks_if_on_border(Vector3i local_pos)
    {
        // Placeholder for neighbor notification logic
        // This would check if the modified voxel is on a chunk boundary
        // and notify adjacent chunks if needed
    }

    int Chunk::get_voxel_material_category_id(Vector3i local_pos)
    {
        if (local_pos.x < 0 || local_pos.x >= CHUNK_SIZE ||
            local_pos.y < 0 || local_pos.y >= CHUNK_SIZE ||
            local_pos.z < 0 || local_pos.z >= CHUNK_SIZE)
        {
            // Default to AIR for out-of-bounds voxels
            return static_cast<int>(MaterialCategory::AIR);
        }

        // Determine material category based on voxel type
        int type = voxels[local_pos.x][local_pos.y][local_pos.z].get_type();

        switch (type)
        {
        case Voxel::AIR:
            return static_cast<int>(MaterialCategory::AIR);
        case Voxel::WATER:
            return static_cast<int>(MaterialCategory::LIQUID);
        default:
            // Default to SOLID for most voxel types
            return static_cast<int>(MaterialCategory::SOLID);
        }
    }

} // namespace voxel_engine