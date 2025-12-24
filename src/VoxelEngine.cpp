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

// Godot includes
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;
using namespace voxel_engine;

namespace voxel_engine {

void VoxelEngine::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_voxel_generator_node", "node"), &VoxelEngine::set_voxel_generator_node);
	ClassDB::bind_method(D_METHOD("get_voxel_generator"), &VoxelEngine::get_voxel_generator);
	ClassDB::bind_method(D_METHOD("set_voxel", "position", "type"), &VoxelEngine::set_voxel);
	ClassDB::bind_method(D_METHOD("get_voxel", "position"), &VoxelEngine::get_voxel);
	ClassDB::bind_method(D_METHOD("get_total_voxel_count"), &VoxelEngine::get_total_voxel_count);
}

VoxelEngine::VoxelEngine() = default;

VoxelEngine::~VoxelEngine() {
	voxel_generator = nullptr; // non-owning
}

void VoxelEngine::_init() {
	// no-op
}

void VoxelEngine::set_voxel_generator_node(Node *node) {
	if (!node) {
		voxel_generator = nullptr;
		UtilityFunctions::print("VoxelEngine: cleared voxel_generator reference");
		return;
	}
	VoxelGenerator *vg = Object::cast_to<VoxelGenerator>(node);
	if (!vg) {
		UtilityFunctions::push_error("VoxelEngine: provided node is not a VoxelGenerator");
		return;
	}
	voxel_generator = vg;
	UtilityFunctions::print("VoxelEngine: bound to VoxelGenerator node");
}

VoxelGenerator *VoxelEngine::get_voxel_generator() const {
	return voxel_generator;
}

void VoxelEngine::set_voxel(const Vector3i &position, int type) {
	if (!voxel_generator) {
		UtilityFunctions::push_error("VoxelEngine::set_voxel: no VoxelGenerator bound");
		return;
	}
	// Best-effort: use build/dig to approximate per-voxel change
	voxel_generator->build_sphere(Vector3(static_cast<float>(position.x), static_cast<float>(position.y), static_cast<float>(position.z)), 0.5f, (type == 0 ? -1.0f : 1.0f));
}

int VoxelEngine::get_voxel(const Vector3i &position) const {
	if (!voxel_generator) {
		UtilityFunctions::push_error("VoxelEngine::get_voxel: no VoxelGenerator bound");
		return 0; // AIR
	}
	// Use VoxelGenerator sampling API if available
	return voxel_generator->get_voxel_at(position);
}

int64_t VoxelEngine::get_total_voxel_count() const {
	if (!voxel_generator)
		return 0;
	Vector3i world_size = voxel_generator->get_world_size();
	int chunk_size = voxel_generator->get_chunk_size();
	int64_t total_x = static_cast<int64_t>(world_size.x) * chunk_size;
	int64_t total_y = static_cast<int64_t>(world_size.y) * chunk_size;
	int64_t total_z = static_cast<int64_t>(world_size.z) * chunk_size;
	return total_x * total_y * total_z;
}

} // namespace voxel_engine