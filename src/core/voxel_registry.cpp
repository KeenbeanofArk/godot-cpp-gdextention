/**************************************************************************/
/*  voxel_registry.cpp                                                    */
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

#include "voxel_registry.h"

// Godot includes
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace voxel_engine {

// Static member initialization
Ref<VoxelRegistry> VoxelRegistry::default_instance;

void VoxelRegistry::_bind_methods() {
	// Static methods
	ClassDB::bind_static_method("VoxelRegistry", D_METHOD("get_default"), &VoxelRegistry::get_default);
	ClassDB::bind_static_method("VoxelRegistry", D_METHOD("set_default", "registry"), &VoxelRegistry::set_default);

	// Registration methods
	ClassDB::bind_method(D_METHOD("register_voxel", "type_id", "name", "category", "property_flags", "hardness", "light_emission", "material_id"), &VoxelRegistry::register_voxel);
	ClassDB::bind_method(D_METHOD("unregister_voxel", "type_id"), &VoxelRegistry::unregister_voxel);
	ClassDB::bind_method(D_METHOD("clear"), &VoxelRegistry::clear);

	// Query methods
	ClassDB::bind_method(D_METHOD("has_definition", "type_id"), &VoxelRegistry::has_definition);
	ClassDB::bind_method(D_METHOD("get_definition_dict", "type_id"), &VoxelRegistry::get_definition_dict);

	// Property queries
	ClassDB::bind_method(D_METHOD("is_solid", "type_id"), &VoxelRegistry::is_solid);
	ClassDB::bind_method(D_METHOD("is_transparent", "type_id"), &VoxelRegistry::is_transparent);
	ClassDB::bind_method(D_METHOD("is_liquid", "type_id"), &VoxelRegistry::is_liquid);
	ClassDB::bind_method(D_METHOD("is_collidable", "type_id"), &VoxelRegistry::is_collidable);
	ClassDB::bind_method(D_METHOD("is_emissive", "type_id"), &VoxelRegistry::is_emissive);
	ClassDB::bind_method(D_METHOD("has_property", "type_id", "flag"), &VoxelRegistry::has_property);

	// Property getters
	ClassDB::bind_method(D_METHOD("get_voxel_name", "type_id"), &VoxelRegistry::get_voxel_name);
	ClassDB::bind_method(D_METHOD("get_material_category", "type_id"), &VoxelRegistry::get_material_category);
	ClassDB::bind_method(D_METHOD("get_hardness", "type_id"), &VoxelRegistry::get_hardness);
	ClassDB::bind_method(D_METHOD("get_light_emission", "type_id"), &VoxelRegistry::get_light_emission);
	ClassDB::bind_method(D_METHOD("get_material_id", "type_id"), &VoxelRegistry::get_material_id);
	ClassDB::bind_method(D_METHOD("get_property_flags", "type_id"), &VoxelRegistry::get_property_flags);

	// Enumeration
	ClassDB::bind_method(D_METHOD("get_all_type_ids"), &VoxelRegistry::get_all_type_ids);
	ClassDB::bind_method(D_METHOD("get_voxel_count"), &VoxelRegistry::get_voxel_count);

	// Bind MaterialCategory enum constants for GDScript
	BIND_CONSTANT(MATERIAL_CATEGORY_AIR);
	BIND_CONSTANT(MATERIAL_CATEGORY_GAS);
	BIND_CONSTANT(MATERIAL_CATEGORY_LIQUID);
	BIND_CONSTANT(MATERIAL_CATEGORY_SOLID);

	// Bind property flag constants
	BIND_CONSTANT(VOXEL_PROPERTY_NONE);
	BIND_CONSTANT(VOXEL_PROPERTY_TRANSPARENT);
	BIND_CONSTANT(VOXEL_PROPERTY_LIQUID);
	BIND_CONSTANT(VOXEL_PROPERTY_FOLIAGE);
	BIND_CONSTANT(VOXEL_PROPERTY_EMISSIVE);
	BIND_CONSTANT(VOXEL_PROPERTY_COLLIDABLE);
	BIND_CONSTANT(VOXEL_PROPERTY_OPAQUE);
	BIND_CONSTANT(VOXEL_PROPERTY_SOLID);
}

VoxelRegistry::VoxelRegistry() {
	register_default_voxels();
}

VoxelRegistry::~VoxelRegistry() {
}

void VoxelRegistry::register_default_voxels() {
	// AIR - transparent, non-solid, non-collidable
	definitions[VoxelType::AIR] = VoxelDefinition(
			VoxelType::AIR, "Air", MaterialCategory::AIR,
			VOXEL_PROPERTY_TRANSPARENT, 0.0f, 0.0f, 0);

	// DIRT - solid, opaque, collidable
	definitions[VoxelType::DIRT] = VoxelDefinition(
			VoxelType::DIRT, "Dirt", MaterialCategory::SOLID,
			VOXEL_PROPERTY_SOLID | VOXEL_PROPERTY_OPAQUE | VOXEL_PROPERTY_COLLIDABLE,
			0.5f, 0.0f, 1);

	// GRASS - solid, opaque, collidable
	definitions[VoxelType::GRASS] = VoxelDefinition(
			VoxelType::GRASS, "Grass", MaterialCategory::SOLID,
			VOXEL_PROPERTY_SOLID | VOXEL_PROPERTY_OPAQUE | VOXEL_PROPERTY_COLLIDABLE,
			0.5f, 0.0f, 2);

	// STONE - solid, opaque, collidable, harder
	definitions[VoxelType::STONE] = VoxelDefinition(
			VoxelType::STONE, "Stone", MaterialCategory::SOLID,
			VOXEL_PROPERTY_SOLID | VOXEL_PROPERTY_OPAQUE | VOXEL_PROPERTY_COLLIDABLE,
			1.5f, 0.0f, 3);

	// WATER - liquid, transparent, non-collidable
	definitions[VoxelType::WATER] = VoxelDefinition(
			VoxelType::WATER, "Water", MaterialCategory::LIQUID,
			VOXEL_PROPERTY_TRANSPARENT | VOXEL_PROPERTY_LIQUID,
			0.0f, 0.0f, 4);

	// SAND - solid, opaque, collidable
	definitions[VoxelType::SAND] = VoxelDefinition(
			VoxelType::SAND, "Sand", MaterialCategory::SOLID,
			VOXEL_PROPERTY_SOLID | VOXEL_PROPERTY_OPAQUE | VOXEL_PROPERTY_COLLIDABLE,
			0.4f, 0.0f, 5);

	// LAVA - liquid, emissive, collidable (damages)
	definitions[VoxelType::LAVA] = VoxelDefinition(
			VoxelType::LAVA, "Lava", MaterialCategory::LIQUID,
			VOXEL_PROPERTY_LIQUID | VOXEL_PROPERTY_EMISSIVE | VOXEL_PROPERTY_COLLIDABLE,
			0.0f, 1.0f, 6);

	// GOLD - solid, opaque, collidable, hard
	definitions[VoxelType::GOLD] = VoxelDefinition(
			VoxelType::GOLD, "Gold Ore", MaterialCategory::SOLID,
			VOXEL_PROPERTY_SOLID | VOXEL_PROPERTY_OPAQUE | VOXEL_PROPERTY_COLLIDABLE,
			3.0f, 0.0f, 7);

	// DIAMOND - solid, opaque, collidable, very hard
	definitions[VoxelType::DIAMOND] = VoxelDefinition(
			VoxelType::DIAMOND, "Diamond Ore", MaterialCategory::SOLID,
			VOXEL_PROPERTY_SOLID | VOXEL_PROPERTY_OPAQUE | VOXEL_PROPERTY_COLLIDABLE,
			5.0f, 0.0f, 8);

	// IRON - solid, opaque, collidable, hard
	definitions[VoxelType::IRON] = VoxelDefinition(
			VoxelType::IRON, "Iron Ore", MaterialCategory::SOLID,
			VOXEL_PROPERTY_SOLID | VOXEL_PROPERTY_OPAQUE | VOXEL_PROPERTY_COLLIDABLE,
			2.0f, 0.0f, 9);

	// COAL - solid, opaque, collidable
	definitions[VoxelType::COAL] = VoxelDefinition(
			VoxelType::COAL, "Coal Ore", MaterialCategory::SOLID,
			VOXEL_PROPERTY_SOLID | VOXEL_PROPERTY_OPAQUE | VOXEL_PROPERTY_COLLIDABLE,
			1.0f, 0.0f, 10);
}

Ref<VoxelRegistry> VoxelRegistry::get_default() {
	if (!default_instance.is_valid()) {
		default_instance.instantiate();
	}
	return default_instance;
}

void VoxelRegistry::set_default(const Ref<VoxelRegistry> &p_registry) {
	default_instance = p_registry;
}

void VoxelRegistry::register_voxel(int type_id, const String &name, int category,
		unsigned int property_flags, float hardness, float light_emission, int material_id) {
	definitions[type_id] = VoxelDefinition(
			type_id, name, static_cast<MaterialCategory>(category),
			property_flags, hardness, light_emission, material_id);
}

void VoxelRegistry::unregister_voxel(int type_id) {
	definitions.erase(type_id);
}

void VoxelRegistry::clear() {
	definitions.clear();
}

bool VoxelRegistry::has_definition(int type_id) const {
	return definitions.has(type_id);
}

Dictionary VoxelRegistry::get_definition_dict(int type_id) const {
	Dictionary result;
	if (definitions.has(type_id)) {
		const VoxelDefinition &def = definitions[type_id];
		result["type_id"] = def.type_id;
		result["name"] = def.name;
		result["category"] = static_cast<int>(def.category);
		result["property_flags"] = def.property_flags;
		result["hardness"] = def.hardness;
		result["light_emission"] = def.light_emission;
		result["material_id"] = def.material_id;
	}
	return result;
}

const VoxelDefinition *VoxelRegistry::get_definition(int type_id) const {
	if (definitions.has(type_id)) {
		return &definitions[type_id];
	}
	return nullptr;
}

bool VoxelRegistry::is_solid(int type_id) const {
	if (definitions.has(type_id)) {
		return definitions[type_id].is_solid();
	}
	return false;
}

bool VoxelRegistry::is_transparent(int type_id) const {
	if (definitions.has(type_id)) {
		return definitions[type_id].is_transparent();
	}
	return true; // Unknown types are transparent by default
}

bool VoxelRegistry::is_liquid(int type_id) const {
	if (definitions.has(type_id)) {
		return definitions[type_id].is_liquid();
	}
	return false;
}

bool VoxelRegistry::is_collidable(int type_id) const {
	if (definitions.has(type_id)) {
		return definitions[type_id].is_collidable();
	}
	return false;
}

bool VoxelRegistry::is_emissive(int type_id) const {
	if (definitions.has(type_id)) {
		return definitions[type_id].is_emissive();
	}
	return false;
}

bool VoxelRegistry::has_property(int type_id, unsigned int flag) const {
	if (definitions.has(type_id)) {
		return definitions[type_id].has_property(flag);
	}
	return false;
}

String VoxelRegistry::get_voxel_name(int type_id) const {
	if (definitions.has(type_id)) {
		return definitions[type_id].name;
	}
	return "Unknown";
}

int VoxelRegistry::get_material_category(int type_id) const {
	if (definitions.has(type_id)) {
		return static_cast<int>(definitions[type_id].category);
	}
	return static_cast<int>(MaterialCategory::AIR);
}

float VoxelRegistry::get_hardness(int type_id) const {
	if (definitions.has(type_id)) {
		return definitions[type_id].hardness;
	}
	return 0.0f;
}

float VoxelRegistry::get_light_emission(int type_id) const {
	if (definitions.has(type_id)) {
		return definitions[type_id].light_emission;
	}
	return 0.0f;
}

int VoxelRegistry::get_material_id(int type_id) const {
	if (definitions.has(type_id)) {
		return definitions[type_id].material_id;
	}
	return 0;
}

unsigned int VoxelRegistry::get_property_flags(int type_id) const {
	if (definitions.has(type_id)) {
		return definitions[type_id].property_flags;
	}
	return VOXEL_PROPERTY_NONE;
}

Array VoxelRegistry::get_all_type_ids() const {
	Array result;
	for (const KeyValue<int, VoxelDefinition> &kv : definitions) {
		result.push_back(kv.key);
	}
	return result;
}

int VoxelRegistry::get_voxel_count() const {
	return definitions.size();
}

} // namespace voxel_engine
