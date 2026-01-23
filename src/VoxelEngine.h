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

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/vector3i.hpp>

using namespace godot;

namespace voxel_engine {

class VoxelGenerator;

class VoxelEngine : public Node {
	GDCLASS(VoxelEngine, Node);

private:
	VoxelGenerator *voxel_generator = nullptr; // owning raw pointer

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	VoxelEngine();
	~VoxelEngine() override;

	void _init();

	// Factory method: create and own a VoxelGenerator
	VoxelGenerator *create_generator();

	// Cleanup: explicitly destroy the owned generator
	void destroy_generator();

	// Get the owned generator
	VoxelGenerator *get_voxel_generator() const;

	// Deprecated: use create_generator() instead. Kept for backward compatibility.
	void set_voxel_generator_node(Node *node);

	// Simple proxies
	void set_voxel(const Vector3i &position, int type);
	int get_voxel(const Vector3i &position) const;
	int64_t get_total_voxel_count() const;
};

} // namespace voxel_engine

#endif // VOXEL_ENGINE_H