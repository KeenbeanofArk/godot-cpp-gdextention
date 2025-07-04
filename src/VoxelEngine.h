/**************************************************************************/
/*  VoxelEngine.h                                                         */
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

#ifndef VOXEL_ENGINE_H
#define VOXEL_ENGINE_H

// We don't need windows.h in this example plugin but many others do, and it can
// lead to annoying situations due to the ton of macros it defines.
// So we include it and make sure CI warns us if we use something that conflicts
// with a Windows define.
#ifdef WIN32
#include <windows.h>
#endif

// Godot includes
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/vector3i.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/node3d.hpp>

using namespace godot;

namespace voxel_engine
{
    class Chunk;
    class Voxel;
    class VoxelGenerator;
    class BiomeGenerator;

    class VoxelEngine : public Node3D
    {
        GDCLASS(VoxelEngine, Node3D);

    protected:
        static void _bind_methods();

    public:
        VoxelEngine();
        ~VoxelEngine();

        void _init(); // Called when the object is instantiated

        // Core engine methods
        void initialize();
        void generate_voxel_world();
        void update_voxel_world();

        // Voxel manipulation methods
        void set_voxel(const Vector3i &position, int type);
        int get_voxel(const Vector3i &position);

        // Property getters
        VoxelGenerator *get_voxel_generator() const { return voxel_generator; }
        Ref<voxel_engine::BiomeGenerator> biome_generator;

        // Chunk management methods
        Chunk *get_chunk(const Vector3i &position);
        Chunk *create_chunk(const Vector3i &position);
        void destroy_chunk(const Vector3i &position);

    private:
        // Components
        VoxelGenerator *voxel_generator;

        // Chunk storage
        HashMap<Vector3i, Chunk *> chunks;
    };

} // namespace voxel_engine

#endif // VOXEL_ENGINE_H