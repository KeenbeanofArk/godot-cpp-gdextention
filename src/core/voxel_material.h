/**************************************************************************/
/*  voxel_material.h                                                      */
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

#ifndef VOXEL_MATERIAL_H
#define VOXEL_MATERIAL_H

// Godot includes
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/string.hpp>

using namespace godot;

namespace voxel_engine {

/// @brief VoxelMaterial - Defines visual properties for a voxel type.
/// This is a placeholder implementation for future texture atlas integration.
class VoxelMaterial : public Resource {
	GDCLASS(VoxelMaterial, Resource);

private:
	int material_id; // Unique material identifier
	String name; // Human-readable name
	Color color_tint; // Base color tint for the material
	float emission_strength; // Emission intensity (0.0 = none, 1.0 = full)
	float roughness; // Surface roughness (0.0 = smooth, 1.0 = rough)
	float metallic; // Metallic property (0.0 = dielectric, 1.0 = metal)

	// Placeholder for future texture atlas integration
	int texture_atlas_index; // Index into texture atlas (placeholder)

protected:
	static void _bind_methods();

public:
	VoxelMaterial();
	~VoxelMaterial();

	// Material ID
	void set_material_id(int p_id);
	int get_material_id() const;

	// Name
	void set_material_name(const String &p_name);
	String get_material_name() const;

	// Color tint
	void set_color_tint(const Color &p_color);
	Color get_color_tint() const;

	// Emission
	void set_emission_strength(float p_strength);
	float get_emission_strength() const;

	// PBR properties
	void set_roughness(float p_roughness);
	float get_roughness() const;

	void set_metallic(float p_metallic);
	float get_metallic() const;

	// Texture atlas (placeholder)
	void set_texture_atlas_index(int p_index);
	int get_texture_atlas_index() const;
};

/// @brief VoxelMaterialLibrary - Manages a collection of VoxelMaterials.
/// Provides lookup by material_id for mesh generation.
class VoxelMaterialLibrary : public Resource {
	GDCLASS(VoxelMaterialLibrary, Resource);

private:
	HashMap<int, Ref<VoxelMaterial>> materials;
	static Ref<VoxelMaterialLibrary> default_instance;

	void register_default_materials();

protected:
	static void _bind_methods();

public:
	VoxelMaterialLibrary();
	~VoxelMaterialLibrary();

	// Static singleton accessor
	static Ref<VoxelMaterialLibrary> get_default();
	static void set_default(const Ref<VoxelMaterialLibrary> &p_library);

	// Material management
	void add_material(const Ref<VoxelMaterial> &p_material);
	void remove_material(int material_id);
	void clear();

	// Queries
	bool has_material(int material_id) const;
	Ref<VoxelMaterial> get_material(int material_id) const;
	Array get_all_material_ids() const;
	int get_material_count() const;

	// Convenience getters
	Color get_color_for_material(int material_id) const;
	float get_emission_for_material(int material_id) const;
};

} // namespace voxel_engine

#endif // VOXEL_MATERIAL_H
