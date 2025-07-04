/**************************************************************************/
/*  VoxelEngine.cpp                                                       */
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

#include "VoxelEngine.h"
#include "VoxelGenerator.h"
#include "register_types.h"
#include "core/chunk.h"
#include "core/voxel.h"
#include "Constants.h"

// Godot includes
#include <godot_cpp/variant/vector3i.hpp>
#include <godot_cpp/core/class_db.hpp>

namespace voxel_engine
{

    void VoxelEngine::_bind_methods()
    {
        // Using the Godot 4 GDExtension approach for registering methods
        ClassDB::bind_method(D_METHOD("initialize"), &VoxelEngine::initialize);
        ClassDB::bind_method(D_METHOD("generate_voxel_world"), &VoxelEngine::generate_voxel_world);
        ClassDB::bind_method(D_METHOD("update_voxel_world"), &VoxelEngine::update_voxel_world);
        ClassDB::bind_method(D_METHOD("set_voxel", "position", "type"), &VoxelEngine::set_voxel);
        ClassDB::bind_method(D_METHOD("get_voxel", "position"), &VoxelEngine::get_voxel);
        ClassDB::bind_method(D_METHOD("create_chunk", "position"), &VoxelEngine::create_chunk);
        ClassDB::bind_method(D_METHOD("destroy_chunk", "position"), &VoxelEngine::destroy_chunk);

        // Bind property getters
        ClassDB::bind_method(D_METHOD("get_voxel_generator"), &VoxelEngine::get_voxel_generator);

        // Add properties
        ClassDB::add_property("VoxelEngine", PropertyInfo(Variant::OBJECT, "voxel_generator", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "VoxelGenerator"), "", "get_voxel_generator");
    }

    VoxelEngine::VoxelEngine()
    {
        // Initialize components
        voxel_generator = memnew(VoxelGenerator);
    }

    VoxelEngine::~VoxelEngine()
    {
        // Clean up components
        if (voxel_generator)
        {
            memdelete(voxel_generator);
            voxel_generator = nullptr;
        }

        // Clean up chunks
        for (HashMap<Vector3i, Chunk *>::Iterator it = chunks.begin(); it != chunks.end(); ++it)
        {
            memdelete(it->value);
        }
        chunks.clear();
    }

    void VoxelEngine::_init()
    {
        // Called when the object is first created
    }

    void VoxelEngine::initialize()
    {
        // Initialize the voxel engine
        UtilityFunctions::print("NativeVoxelEngine: Initializing Voxel Engine");

        // In VoxelEngine constructor or initialization
        biome_generator.instantiate();
    }

    void VoxelEngine::generate_voxel_world()
    {
        // Generate the initial voxel world
        UtilityFunctions::print("NativeVoxelEngine: Generating Voxel World");

        // Example: Generate a few chunks around the origin
        for (int x = -2; x <= 2; x++)
        {
            for (int z = -2; z <= 2; z++)
            {
                Vector3i chunk_pos(x, 0, z);
                create_chunk(chunk_pos);
            }
        }
    }

    void VoxelEngine::update_voxel_world()
    {
        // Update the voxel world state
        // This would typically be called each frame
    }

    void VoxelEngine::set_voxel(const Vector3i &position, int type)
    {
        // Convert world position to chunk position and local position
        Vector3i chunk_pos = Vector3i(
            floor(position.x / (float)Chunk::get_chunk_size()),
            floor(position.y / (float)Chunk::get_chunk_size()),
            floor(position.z / (float)Chunk::get_chunk_size()));

        Vector3i local_pos = Vector3i(
            position.x - chunk_pos.x * Chunk::get_chunk_size(),
            position.y - chunk_pos.y * Chunk::get_chunk_size(),
            position.z - chunk_pos.z * Chunk::get_chunk_size());

        // Get the chunk
        Chunk *chunk = get_chunk(chunk_pos);
        if (!chunk)
        {
            // Create the chunk if it doesn't exist
            chunk = create_chunk(chunk_pos);
        }

        // Set the voxel
        chunk->set_voxel(local_pos, static_cast<VoxelType>(type));
    }

    int VoxelEngine::get_voxel(const Vector3i &position)
    {
        // Convert world position to chunk position and local position
        Vector3i chunk_pos = Vector3i(
            floor(position.x / (float)Chunk::get_chunk_size()),
            floor(position.y / (float)Chunk::get_chunk_size()),
            floor(position.z / (float)Chunk::get_chunk_size()));

        Vector3i local_pos = Vector3i(
            position.x - chunk_pos.x * Chunk::get_chunk_size(),
            position.y - chunk_pos.y * Chunk::get_chunk_size(),
            position.z - chunk_pos.z * Chunk::get_chunk_size());

        // Get the chunk
        Chunk *chunk = get_chunk(chunk_pos);
        if (!chunk)
        {
            return static_cast<int>(VoxelType::AIR);
        }

        // Get the voxel - now returns Ref<Voxel> instead of Voxel
        Ref<Voxel> voxel = chunk->get_voxel(local_pos);
        if (voxel.is_valid())
        {
            return static_cast<int>(voxel->get_type());
        }
        else
        {
            return static_cast<int>(VoxelType::AIR);
        }
    }

    Chunk *VoxelEngine::get_chunk(const Vector3i &position)
    {
        if (chunks.has(position))
        {
            return chunks[position];
        }
        return nullptr;
    }

    Chunk *VoxelEngine::create_chunk(const Vector3i &position)
    {
        // Check if the chunk already exists
        if (chunks.has(position))
        {
            return chunks[position];
        }

        // Create a new chunk
        Chunk *chunk = memnew(Chunk);
        chunk->position = Vector3(position);
        chunk->set_biome_generator(biome_generator); // Set the biome generator for the chunk
        chunk->chunk_id = position.x + position.y * 1000 + position.z * 1000000; // Unique ID based on position

        // Generate the chunk
        chunk->generate();

        // Add the chunk to the map
        chunks[position] = chunk;

        return chunk;
    }

    void VoxelEngine::destroy_chunk(const Vector3i &position)
    {
        // Check if the chunk exists
        if (chunks.has(position))
        {
            // Delete the chunk
            memdelete(chunks[position]);
            chunks.erase(position);
        }
    }

} // namespace voxel_engine