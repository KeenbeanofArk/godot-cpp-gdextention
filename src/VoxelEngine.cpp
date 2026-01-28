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
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;
using namespace voxel_engine;

namespace voxel_engine {

void VoxelEngine::_bind_methods() {
	// Factory methods (primary API)
	ClassDB::bind_method(D_METHOD("create_generator"), &VoxelEngine::create_generator);
	ClassDB::bind_method(D_METHOD("destroy_generator"), &VoxelEngine::destroy_generator);
	ClassDB::bind_method(D_METHOD("get_voxel_generator"), &VoxelEngine::get_voxel_generator);
	ClassDB::bind_method(D_METHOD("load_terrain", "terrain_name"), &VoxelEngine::load_terrain);

	// Deprecated methods (kept for backward compatibility)
	ClassDB::bind_method(D_METHOD("set_voxel_generator_node", "node"), &VoxelEngine::set_voxel_generator_node);

	// Proxy methods
	ClassDB::bind_method(D_METHOD("set_voxel", "position", "type"), &VoxelEngine::set_voxel);
	ClassDB::bind_method(D_METHOD("get_voxel", "position"), &VoxelEngine::get_voxel);
	ClassDB::bind_method(D_METHOD("get_total_voxel_count"), &VoxelEngine::get_total_voxel_count);
}

VoxelEngine::VoxelEngine() = default;

VoxelEngine::~VoxelEngine() {
	// Cleanup of VoxelGenerator is handled in _notification(NOTIFICATION_PREDELETE)
	// This ensures proper Godot lifecycle management during node deletion
}

void VoxelEngine::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_PREDELETE:
			// Explicit cleanup when node is being deleted from the scene tree
			destroy_generator();
			break;
		default:
			break;
	}
}

void VoxelEngine::_init() {
	// no-op
}

VoxelGenerator *VoxelEngine::create_generator() {
	// Destroy any existing generator first
	if (voxel_generator != nullptr) {
		UtilityFunctions::print_verbose("VoxelEngine::create_generator: destroying existing generator");
		destroy_generator();
	}

	// Create new VoxelGenerator and own it
	voxel_generator = memnew(VoxelGenerator);
	voxel_generator->set_name("VoxelGenerator");

	// Add as child to ensure it's part of the scene tree
	add_child(voxel_generator);

	UtilityFunctions::print("VoxelEngine::create_generator: created and added VoxelGenerator as child");
	return voxel_generator;
}

void VoxelEngine::destroy_generator() {
	if (voxel_generator == nullptr) {
		return; // Already destroyed or never created
	}

	// Cancel any ongoing generation
	if (voxel_generator->is_generating()) {
		UtilityFunctions::print_verbose("VoxelEngine::destroy_generator: canceling ongoing generation");
		voxel_generator->cancel_generation();
	}

	// Remove from scene tree
	if (voxel_generator->is_inside_tree()) {
		remove_child(voxel_generator);
	}

	// Delete the generator node (this will also trigger its destructor)
	memdelete(voxel_generator);
	voxel_generator = nullptr;

	UtilityFunctions::print("VoxelEngine::destroy_generator: VoxelGenerator destroyed");
}

VoxelGenerator *VoxelEngine::load_terrain(const String &terrain_name) {
	UtilityFunctions::print(String("[VoxelEngine::load_terrain] Loading terrain: {0}").format(Array::make(terrain_name)));

	// Destroy any existing generator first
	if (voxel_generator != nullptr) {
		UtilityFunctions::print("[VoxelEngine::load_terrain] Destroying existing generator");
		destroy_generator();
	}

	// Load the config resource from res://configs/{terrain_name}.tres
	String config_path = String("res://configs/{0}.tres").format(Array::make(terrain_name));
	Ref<Resource> config_resource = ResourceLoader::get_singleton()->load(config_path);

	if (!config_resource.is_valid()) {
		UtilityFunctions::push_error(String("[VoxelEngine::load_terrain] Failed to load config: {0}").format(Array::make(config_path)));
		// Create generator with defaults even if config fails to load
		return create_generator();
	}

	// Create a new generator
	VoxelGenerator *gen = create_generator();
	if (!gen) {
		UtilityFunctions::push_error("[VoxelEngine::load_terrain] Failed to create generator");
		return nullptr;
	}

	// Call apply_to_voxel_generator() on the config resource if the method exists
	// This applies all terrain-specific properties (world_size, resolution, seeder, etc.)
	if (config_resource->has_method("apply_to_voxel_generator")) {
		UtilityFunctions::print(String("[VoxelEngine::load_terrain] Applying config to generator").format(Array::make()));
		config_resource->call("apply_to_voxel_generator", gen);
	} else {
		UtilityFunctions::push_warning(String("[VoxelEngine::load_terrain] Config resource {0} does not have apply_to_voxel_generator() method").format(Array::make(config_path)));
	}

	UtilityFunctions::print(String("[VoxelEngine::load_terrain] Terrain {0} loaded successfully").format(Array::make(terrain_name)));
	return gen;
}

VoxelGenerator *VoxelEngine::get_voxel_generator() const {
	return voxel_generator;
}

void VoxelEngine::set_voxel_generator_node(Node *node) {
	// DEPRECATED: This method is deprecated. Use create_generator() instead.
	// Kept for backward compatibility only.
	UtilityFunctions::push_warning("VoxelEngine::set_voxel_generator_node is deprecated. Use create_generator() instead.");

	if (!node) {
		destroy_generator();
		UtilityFunctions::print("VoxelEngine: cleared voxel_generator reference");
		return;
	}

	VoxelGenerator *vg = Object::cast_to<VoxelGenerator>(node);
	if (!vg) {
		UtilityFunctions::push_error("VoxelEngine::set_voxel_generator_node: provided node is not a VoxelGenerator");
		return;
	}

	// If we already own a generator, destroy it first
	if (voxel_generator != nullptr && voxel_generator != vg) {
		destroy_generator();
	}

	// Take ownership of the externally-created VoxelGenerator node
	voxel_generator = vg;
	if (!voxel_generator->is_inside_tree()) {
		add_child(voxel_generator);
	}
	UtilityFunctions::print("VoxelEngine: bound to VoxelGenerator node (via deprecated API)");
}

void VoxelEngine::set_voxel(const Vector3i &position, int type) {
	if (voxel_generator == nullptr) {
		UtilityFunctions::push_error("VoxelEngine::set_voxel: no VoxelGenerator available (call create_generator first)");
		return;
	}
	// Best-effort: use build/dig to approximate per-voxel change
	voxel_generator->build_sphere(Vector3(static_cast<float>(position.x), static_cast<float>(position.y), static_cast<float>(position.z)), 0.5f, (type == 0 ? -1.0f : 1.0f));
}

int VoxelEngine::get_voxel(const Vector3i &position) const {
	if (voxel_generator == nullptr) {
		UtilityFunctions::push_error("VoxelEngine::get_voxel: no VoxelGenerator available (call create_generator first)");
		return 0; // AIR
	}
	// Use VoxelGenerator sampling API if available
	return voxel_generator->get_voxel_at(position);
}

int64_t VoxelEngine::get_total_voxel_count() const {
	if (voxel_generator == nullptr)
		return 0;
	Vector3i world_size = voxel_generator->get_world_size();
	int chunk_size = voxel_generator->get_chunk_size();
	int64_t total_x = static_cast<int64_t>(world_size.x) * chunk_size;
	int64_t total_y = static_cast<int64_t>(world_size.y) * chunk_size;
	int64_t total_z = static_cast<int64_t>(world_size.z) * chunk_size;
	return total_x * total_y * total_z;
}

} // namespace voxel_engine