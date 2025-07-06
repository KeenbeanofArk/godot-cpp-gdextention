/**************************************************************************/
/*  VoxelGenerator.h                                                      */
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

#ifndef VOXEL_GENERATOR_H
#define VOXEL_GENERATOR_H

// We don't need windows.h in this example plugin but many others do, and it can
// lead to annoying situations due to the ton of macros it defines.
// So we include it and make sure CI warns us if we use something that conflicts
// with a Windows define.
#ifdef WIN32
#include <windows.h>
#endif

#include "core/chunk.h"
#include "core/voxel.h"
#include "Constants.h"

#include <godot_cpp/classes/fast_noise_lite.hpp>
#include <godot_cpp/classes/immediate_mesh.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/vector3.hpp>

using namespace godot;

namespace voxel_engine {

class VoxelGenerator : public Node3D {
	GDCLASS(VoxelGenerator, Node3D)

private:
	int gen_size_x = 1; // Size of the generator in the X dimension based on the chunk size
	int gen_size_y = 1; // Size of the generator in the Y dimension based on the chunk size
	int gen_size_z = 1; // Size of the generator in the Z dimension based on the chunk size
	inline static Vector3i world_size = Vector3i(1, 1, 1);
	int resolution = 1;
	float cutoff = 0.1f;
	bool show_centers = false;
	bool show_grid = false;
	int seeder = 1234;
	bool auto_generate = true;
	bool vertex_limit = false; // Limit the number of vertices generated

	// Debug properties
	bool debug_mode = true;
	bool visualize_noise_values = true;
	int debug_verbosity = 1;

	// Add a container for chunks, e.g.:
	std::vector<Chunk *> chunks;
	int chunk_size = Constants::DEFAULT_CHUNK_SIZE; // Default value from voxel_constants.h

	const bool object_instance_binding_set_by_parent_constructor;
	bool has_object_instance_binding() const;

protected:
	static void _bind_methods();
	// This method is called to notify the object of various events.
	void _notification(int p_what);

public:
	VoxelGenerator();
	~VoxelGenerator();

	void set_world_size(const Vector3i &value);
 	Vector3i get_world_size() const;

	void set_gen_size_x(int value);
	int get_gen_size_x() const;

	void set_gen_size_y(int value);
	int get_gen_size_y() const;

	void set_gen_size_z(int value);
	int get_gen_size_z() const;

	void set_resolution(int value);
	int get_resolution() const;

	void set_cutoff(float value);
	float get_cutoff() const;

	void set_randomizer(bool value);
	bool get_randomizer() const;

	void set_show_centers(bool value);
	bool get_show_centers() const;

	void set_show_grid(bool value);
	bool get_show_grid() const;

	void set_seeder(int value);
	int get_seeder() const;

	void set_auto_generate(bool value);
	bool get_auto_generate() const;

	void set_vertex_limit(bool value);
	bool get_vertex_limit() const;

	void set_chunk_size(int value);
    int get_chunk_size() const;

	void reset();

	void generate();

	// Debug methods
	void set_debug_mode(bool p_enabled);
	bool get_debug_mode() const;

	void set_visualize_noise_values(bool p_enabled);
	bool get_visualize_noise_values() const;

	void set_debug_verbosity(int p_level);
	int get_debug_verbosity() const;

	void debug_print_state();
	void debug_draw_noise_slice(float y_level);
	void log_message(const String &message, int verbosity_level = 1);

	bool is_object_binding_set_by_parent_constructor() const;

private:
	void remove_children();
	void randomize_seed();
	Vector<Vector3> create_cube_vertices(const Vector3 &pos);
	void add_cubes_vertices(Ref<ImmediateMesh> mesh, const Vector<Vector3> &vertices);
	std::vector<float> get_cube_values(Ref<FastNoiseLite> noise, const Vector<Vector3> &cube_vertices);
	int get_lookup_index(const std::vector<float> &cube_values, float cutoff);
	Vector3 interpolate(const Vector3 &vertex_1, float value_1, const Vector3 &vertex_2, float value_2);
	void add_cube_edges(Ref<ImmediateMesh> mesh, const std::vector<Vector3> &v);
	void calculate_world_size();

	// Debug helpers
	void create_debug_visualization();
	void visualize_noise_field();

	// Optionally, add helpers to manage chunks/voxels
	void create_chunks();
	void fill_chunk_with_voxels(Chunk *chunk);

	bool is_instance_valid(Chunk *chunk) const;
};
} // namespace voxel_engine

#endif // VOXEL_GENERATOR_H