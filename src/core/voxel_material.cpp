/**************************************************************************/
/*  voxel_material.cpp                                                    */
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

#include "voxel_material.h"

// Godot includes
#include <godot_cpp/core/class_db.hpp>

namespace voxel_engine {

// ============================================================================
// VoxelMaterial Implementation
// ============================================================================

void VoxelMaterial::_bind_methods() {
	// Material ID
	ClassDB::bind_method(D_METHOD("set_material_id", "id"), &VoxelMaterial::set_material_id);
	ClassDB::bind_method(D_METHOD("get_material_id"), &VoxelMaterial::get_material_id);

	// Name
	ClassDB::bind_method(D_METHOD("set_material_name", "name"), &VoxelMaterial::set_material_name);
	ClassDB::bind_method(D_METHOD("get_material_name"), &VoxelMaterial::get_material_name);

	// Color tint
	ClassDB::bind_method(D_METHOD("set_color_tint", "color"), &VoxelMaterial::set_color_tint);
	ClassDB::bind_method(D_METHOD("get_color_tint"), &VoxelMaterial::get_color_tint);

	// Emission
	ClassDB::bind_method(D_METHOD("set_emission_strength", "strength"), &VoxelMaterial::set_emission_strength);
	ClassDB::bind_method(D_METHOD("get_emission_strength"), &VoxelMaterial::get_emission_strength);

	// PBR properties
	ClassDB::bind_method(D_METHOD("set_roughness", "roughness"), &VoxelMaterial::set_roughness);
	ClassDB::bind_method(D_METHOD("get_roughness"), &VoxelMaterial::get_roughness);

	ClassDB::bind_method(D_METHOD("set_metallic", "metallic"), &VoxelMaterial::set_metallic);
	ClassDB::bind_method(D_METHOD("get_metallic"), &VoxelMaterial::get_metallic);

	// Texture atlas (placeholder)
	ClassDB::bind_method(D_METHOD("set_texture_atlas_index", "index"), &VoxelMaterial::set_texture_atlas_index);
	ClassDB::bind_method(D_METHOD("get_texture_atlas_index"), &VoxelMaterial::get_texture_atlas_index);

	// Properties
	ADD_GROUP("Material", "");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "material_id"), "set_material_id", "get_material_id");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "material_name"), "set_material_name", "get_material_name");

	ADD_GROUP("Appearance", "");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color_tint"), "set_color_tint", "get_color_tint");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "emission_strength", PROPERTY_HINT_RANGE, "0.0,1.0,0.01"), "set_emission_strength", "get_emission_strength");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "roughness", PROPERTY_HINT_RANGE, "0.0,1.0,0.01"), "set_roughness", "get_roughness");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "metallic", PROPERTY_HINT_RANGE, "0.0,1.0,0.01"), "set_metallic", "get_metallic");

	ADD_GROUP("Texture", "texture_");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "texture_atlas_index"), "set_texture_atlas_index", "get_texture_atlas_index");
}

VoxelMaterial::VoxelMaterial() :
		material_id(0),
		name("Default"),
		color_tint(Color(1.0f, 1.0f, 1.0f, 1.0f)),
		emission_strength(0.0f),
		roughness(0.8f),
		metallic(0.0f),
		texture_atlas_index(0) {
}

VoxelMaterial::~VoxelMaterial() {
}

void VoxelMaterial::set_material_id(int p_id) {
	material_id = p_id;
}

int VoxelMaterial::get_material_id() const {
	return material_id;
}

void VoxelMaterial::set_material_name(const String &p_name) {
	name = p_name;
}

String VoxelMaterial::get_material_name() const {
	return name;
}

void VoxelMaterial::set_color_tint(const Color &p_color) {
	color_tint = p_color;
}

Color VoxelMaterial::get_color_tint() const {
	return color_tint;
}

void VoxelMaterial::set_emission_strength(float p_strength) {
	emission_strength = Math::clamp(p_strength, 0.0f, 1.0f);
}

float VoxelMaterial::get_emission_strength() const {
	return emission_strength;
}

void VoxelMaterial::set_roughness(float p_roughness) {
	roughness = Math::clamp(p_roughness, 0.0f, 1.0f);
}

float VoxelMaterial::get_roughness() const {
	return roughness;
}

void VoxelMaterial::set_metallic(float p_metallic) {
	metallic = Math::clamp(p_metallic, 0.0f, 1.0f);
}

float VoxelMaterial::get_metallic() const {
	return metallic;
}

void VoxelMaterial::set_texture_atlas_index(int p_index) {
	texture_atlas_index = p_index;
}

int VoxelMaterial::get_texture_atlas_index() const {
	return texture_atlas_index;
}

// ============================================================================
// VoxelMaterialLibrary Implementation
// ============================================================================

Ref<VoxelMaterialLibrary> VoxelMaterialLibrary::default_instance;

void VoxelMaterialLibrary::_bind_methods() {
	// Static methods
	ClassDB::bind_static_method("VoxelMaterialLibrary", D_METHOD("get_default"), &VoxelMaterialLibrary::get_default);
	ClassDB::bind_static_method("VoxelMaterialLibrary", D_METHOD("set_default", "library"), &VoxelMaterialLibrary::set_default);

	// Material management
	ClassDB::bind_method(D_METHOD("add_material", "material"), &VoxelMaterialLibrary::add_material);
	ClassDB::bind_method(D_METHOD("remove_material", "material_id"), &VoxelMaterialLibrary::remove_material);
	ClassDB::bind_method(D_METHOD("clear"), &VoxelMaterialLibrary::clear);

	// Queries
	ClassDB::bind_method(D_METHOD("has_material", "material_id"), &VoxelMaterialLibrary::has_material);
	ClassDB::bind_method(D_METHOD("get_material", "material_id"), &VoxelMaterialLibrary::get_material);
	ClassDB::bind_method(D_METHOD("get_all_material_ids"), &VoxelMaterialLibrary::get_all_material_ids);
	ClassDB::bind_method(D_METHOD("get_material_count"), &VoxelMaterialLibrary::get_material_count);

	// Convenience getters
	ClassDB::bind_method(D_METHOD("get_color_for_material", "material_id"), &VoxelMaterialLibrary::get_color_for_material);
	ClassDB::bind_method(D_METHOD("get_emission_for_material", "material_id"), &VoxelMaterialLibrary::get_emission_for_material);
}

VoxelMaterialLibrary::VoxelMaterialLibrary() {
	register_default_materials();
}

VoxelMaterialLibrary::~VoxelMaterialLibrary() {
}

void VoxelMaterialLibrary::register_default_materials() {
	// Create placeholder materials matching VoxelType material_ids

	// 0 - Air (transparent)
	Ref<VoxelMaterial> air;
	air.instantiate();
	air->set_material_id(0);
	air->set_material_name("Air");
	air->set_color_tint(Color(0.0f, 0.0f, 0.0f, 0.0f));
	materials[0] = air;

	// 1 - Dirt (brown)
	Ref<VoxelMaterial> dirt;
	dirt.instantiate();
	dirt->set_material_id(1);
	dirt->set_material_name("Dirt");
	dirt->set_color_tint(Color(0.55f, 0.35f, 0.2f, 1.0f));
	dirt->set_roughness(0.9f);
	materials[1] = dirt;

	// 2 - Grass (green top)
	Ref<VoxelMaterial> grass;
	grass.instantiate();
	grass->set_material_id(2);
	grass->set_material_name("Grass");
	grass->set_color_tint(Color(0.3f, 0.6f, 0.2f, 1.0f));
	grass->set_roughness(0.85f);
	materials[2] = grass;

	// 3 - Stone (gray)
	Ref<VoxelMaterial> stone;
	stone.instantiate();
	stone->set_material_id(3);
	stone->set_material_name("Stone");
	stone->set_color_tint(Color(0.5f, 0.5f, 0.5f, 1.0f));
	stone->set_roughness(0.7f);
	materials[3] = stone;

	// 4 - Water (blue, transparent)
	Ref<VoxelMaterial> water;
	water.instantiate();
	water->set_material_id(4);
	water->set_material_name("Water");
	water->set_color_tint(Color(0.2f, 0.4f, 0.8f, 0.6f));
	water->set_roughness(0.1f);
	materials[4] = water;

	// 5 - Sand (tan)
	Ref<VoxelMaterial> sand;
	sand.instantiate();
	sand->set_material_id(5);
	sand->set_material_name("Sand");
	sand->set_color_tint(Color(0.85f, 0.75f, 0.5f, 1.0f));
	sand->set_roughness(0.95f);
	materials[5] = sand;

	// 6 - Lava (orange, emissive)
	Ref<VoxelMaterial> lava;
	lava.instantiate();
	lava->set_material_id(6);
	lava->set_material_name("Lava");
	lava->set_color_tint(Color(1.0f, 0.3f, 0.0f, 1.0f));
	lava->set_emission_strength(1.0f);
	lava->set_roughness(0.3f);
	materials[6] = lava;

	// 7 - Gold Ore (golden)
	Ref<VoxelMaterial> gold;
	gold.instantiate();
	gold->set_material_id(7);
	gold->set_material_name("Gold Ore");
	gold->set_color_tint(Color(1.0f, 0.84f, 0.0f, 1.0f));
	gold->set_metallic(0.8f);
	gold->set_roughness(0.4f);
	materials[7] = gold;

	// 8 - Diamond Ore (cyan)
	Ref<VoxelMaterial> diamond;
	diamond.instantiate();
	diamond->set_material_id(8);
	diamond->set_material_name("Diamond Ore");
	diamond->set_color_tint(Color(0.4f, 0.9f, 1.0f, 1.0f));
	diamond->set_roughness(0.2f);
	materials[8] = diamond;

	// 9 - Iron Ore (rust/silver)
	Ref<VoxelMaterial> iron;
	iron.instantiate();
	iron->set_material_id(9);
	iron->set_material_name("Iron Ore");
	iron->set_color_tint(Color(0.7f, 0.55f, 0.45f, 1.0f));
	iron->set_metallic(0.6f);
	iron->set_roughness(0.6f);
	materials[9] = iron;

	// 10 - Coal Ore (black)
	Ref<VoxelMaterial> coal;
	coal.instantiate();
	coal->set_material_id(10);
	coal->set_material_name("Coal Ore");
	coal->set_color_tint(Color(0.15f, 0.15f, 0.15f, 1.0f));
	coal->set_roughness(0.8f);
	materials[10] = coal;
}

Ref<VoxelMaterialLibrary> VoxelMaterialLibrary::get_default() {
	if (!default_instance.is_valid()) {
		default_instance.instantiate();
	}
	return default_instance;
}

void VoxelMaterialLibrary::set_default(const Ref<VoxelMaterialLibrary> &p_library) {
	default_instance = p_library;
}

void VoxelMaterialLibrary::add_material(const Ref<VoxelMaterial> &p_material) {
	if (p_material.is_valid()) {
		materials[p_material->get_material_id()] = p_material;
	}
}

void VoxelMaterialLibrary::remove_material(int material_id) {
	materials.erase(material_id);
}

void VoxelMaterialLibrary::clear() {
	materials.clear();
}

bool VoxelMaterialLibrary::has_material(int material_id) const {
	return materials.has(material_id);
}

Ref<VoxelMaterial> VoxelMaterialLibrary::get_material(int material_id) const {
	if (materials.has(material_id)) {
		return materials[material_id];
	}
	return Ref<VoxelMaterial>();
}

Array VoxelMaterialLibrary::get_all_material_ids() const {
	Array result;
	for (const KeyValue<int, Ref<VoxelMaterial>> &kv : materials) {
		result.push_back(kv.key);
	}
	return result;
}

int VoxelMaterialLibrary::get_material_count() const {
	return materials.size();
}

Color VoxelMaterialLibrary::get_color_for_material(int material_id) const {
	if (materials.has(material_id)) {
		return materials[material_id]->get_color_tint();
	}
	return Color(1.0f, 0.0f, 1.0f, 1.0f); // Magenta for missing materials
}

float VoxelMaterialLibrary::get_emission_for_material(int material_id) const {
	if (materials.has(material_id)) {
		return materials[material_id]->get_emission_strength();
	}
	return 0.0f;
}

} // namespace voxel_engine
