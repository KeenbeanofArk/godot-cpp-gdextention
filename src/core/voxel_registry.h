/**************************************************************************/
/*  voxel_registry.h                                                      */
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

#ifndef VOXEL_REGISTRY_H
#define VOXEL_REGISTRY_H

#include "voxel.h"
#include "voxel_constants.h"

// Godot includes
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>

using namespace godot;

namespace voxel_engine {

class VoxelRegistry : public Resource {
	GDCLASS(VoxelRegistry, Resource);

private:
	HashMap<int, VoxelDefinition> definitions;
	static Ref<VoxelRegistry> default_instance;

	void register_default_voxels();

protected:
	static void _bind_methods();

public:
	VoxelRegistry();
	~VoxelRegistry();

	// Static singleton accessor for default registry
	static Ref<VoxelRegistry> get_default();
	static void set_default(const Ref<VoxelRegistry> &p_registry);

	// Registration methods
	void register_voxel(int type_id, const String &name, int category,
			unsigned int property_flags, float hardness, float light_emission, int material_id);
	void unregister_voxel(int type_id);
	void clear();

	// Query methods
	bool has_definition(int type_id) const;
	Dictionary get_definition_dict(int type_id) const;
	const VoxelDefinition *get_definition(int type_id) const;

	// Property queries (convenience methods exposed to GDScript)
	bool is_solid(int type_id) const;
	bool is_transparent(int type_id) const;
	bool is_liquid(int type_id) const;
	bool is_collidable(int type_id) const;
	bool is_emissive(int type_id) const;
	bool has_property(int type_id, unsigned int flag) const;

	// Property getters
	String get_voxel_name(int type_id) const;
	int get_material_category(int type_id) const;
	float get_hardness(int type_id) const;
	float get_light_emission(int type_id) const;
	int get_material_id(int type_id) const;
	unsigned int get_property_flags(int type_id) const;

	// Enumeration
	Array get_all_type_ids() const;
	int get_voxel_count() const;
};

} // namespace voxel_engine

#endif // VOXEL_REGISTRY_H
