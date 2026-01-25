/**************************************************************************/
/*  BiomePreset.h                                                         */
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

#ifndef BIOME_PRESET_H
#define BIOME_PRESET_H

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/typed_array.hpp>

using namespace godot;

namespace voxel_engine {

/// @brief BiomePreset - Resource class wrapping biome configuration for serialization to .tres files
/// Stores all parameters needed to configure a single biome (height range, temperature, humidity, blocks)
class BiomePreset : public Resource {
	GDCLASS(BiomePreset, Resource);

private:
	String biome_name;
	float min_height = -1.0f;
	float max_height = 1.0f;
	float min_temperature = -1.0f;
	float max_temperature = 1.0f;
	float min_humidity = -1.0f;
	float max_humidity = 1.0f;
	TypedArray<int32_t> surface_blocks; // Array of voxel type IDs for surface layer
	TypedArray<int32_t> subsurface_blocks; // Array of voxel type IDs for subsurface layer
	int depth = 4; // Depth of subsurface layer
	int bedrock_block = 1; // STONE by default (VoxelType::STONE = 1)
	int filler_block = 1; // STONE by default

protected:
	static void _bind_methods();

public:
	BiomePreset();
	~BiomePreset();

	// Biome name
	void set_biome_name(const String &p_name);
	String get_biome_name() const;

	// Height range
	void set_min_height(float p_height);
	float get_min_height() const;

	void set_max_height(float p_height);
	float get_max_height() const;

	// Temperature range
	void set_min_temperature(float p_temp);
	float get_min_temperature() const;

	void set_max_temperature(float p_temp);
	float get_max_temperature() const;

	// Humidity range
	void set_min_humidity(float p_humidity);
	float get_min_humidity() const;

	void set_max_humidity(float p_humidity);
	float get_max_humidity() const;

	// Surface blocks array
	void set_surface_blocks(const TypedArray<int32_t> &p_blocks);
	TypedArray<int32_t> get_surface_blocks() const;

	// Subsurface blocks array
	void set_subsurface_blocks(const TypedArray<int32_t> &p_blocks);
	TypedArray<int32_t> get_subsurface_blocks() const;

	// Depth
	void set_depth(int p_depth);
	int get_depth() const;

	// Bedrock block type
	void set_bedrock_block(int p_block);
	int get_bedrock_block() const;

	// Filler block type
	void set_filler_block(int p_block);
	int get_filler_block() const;
};

} // namespace voxel_engine

#endif // BIOME_PRESET_H
