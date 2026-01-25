/**************************************************************************/
/*  BiomePreset.cpp                                                       */
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

#include "BiomePreset.h"

namespace voxel_engine {

void BiomePreset::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_biome_name", "name"), &BiomePreset::set_biome_name);
	ClassDB::bind_method(D_METHOD("get_biome_name"), &BiomePreset::get_biome_name);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "biome_name"), "set_biome_name", "get_biome_name");

	ClassDB::bind_method(D_METHOD("set_min_height", "height"), &BiomePreset::set_min_height);
	ClassDB::bind_method(D_METHOD("get_min_height"), &BiomePreset::get_min_height);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "min_height", PROPERTY_HINT_RANGE, "-256,256,0.5"), "set_min_height", "get_min_height");

	ClassDB::bind_method(D_METHOD("set_max_height", "height"), &BiomePreset::set_max_height);
	ClassDB::bind_method(D_METHOD("get_max_height"), &BiomePreset::get_max_height);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_height", PROPERTY_HINT_RANGE, "-256,256,0.5"), "set_max_height", "get_max_height");

	ClassDB::bind_method(D_METHOD("set_min_temperature", "temp"), &BiomePreset::set_min_temperature);
	ClassDB::bind_method(D_METHOD("get_min_temperature"), &BiomePreset::get_min_temperature);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "min_temperature", PROPERTY_HINT_RANGE, "-1,1,0.1"), "set_min_temperature", "get_min_temperature");

	ClassDB::bind_method(D_METHOD("set_max_temperature", "temp"), &BiomePreset::set_max_temperature);
	ClassDB::bind_method(D_METHOD("get_max_temperature"), &BiomePreset::get_max_temperature);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_temperature", PROPERTY_HINT_RANGE, "-1,1,0.1"), "set_max_temperature", "get_max_temperature");

	ClassDB::bind_method(D_METHOD("set_min_humidity", "humidity"), &BiomePreset::set_min_humidity);
	ClassDB::bind_method(D_METHOD("get_min_humidity"), &BiomePreset::get_min_humidity);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "min_humidity", PROPERTY_HINT_RANGE, "-1,1,0.1"), "set_min_humidity", "get_min_humidity");

	ClassDB::bind_method(D_METHOD("set_max_humidity", "humidity"), &BiomePreset::set_max_humidity);
	ClassDB::bind_method(D_METHOD("get_max_humidity"), &BiomePreset::get_max_humidity);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_humidity", PROPERTY_HINT_RANGE, "-1,1,0.1"), "set_max_humidity", "get_max_humidity");

	ClassDB::bind_method(D_METHOD("set_surface_blocks", "blocks"), &BiomePreset::set_surface_blocks);
	ClassDB::bind_method(D_METHOD("get_surface_blocks"), &BiomePreset::get_surface_blocks);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT32_ARRAY, "surface_blocks"), "set_surface_blocks", "get_surface_blocks");

	ClassDB::bind_method(D_METHOD("set_subsurface_blocks", "blocks"), &BiomePreset::set_subsurface_blocks);
	ClassDB::bind_method(D_METHOD("get_subsurface_blocks"), &BiomePreset::get_subsurface_blocks);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT32_ARRAY, "subsurface_blocks"), "set_subsurface_blocks", "get_subsurface_blocks");

	ClassDB::bind_method(D_METHOD("set_depth", "depth"), &BiomePreset::set_depth);
	ClassDB::bind_method(D_METHOD("get_depth"), &BiomePreset::get_depth);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "depth", PROPERTY_HINT_RANGE, "1,16,1"), "set_depth", "get_depth");

	ClassDB::bind_method(D_METHOD("set_bedrock_block", "block"), &BiomePreset::set_bedrock_block);
	ClassDB::bind_method(D_METHOD("get_bedrock_block"), &BiomePreset::get_bedrock_block);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "bedrock_block"), "set_bedrock_block", "get_bedrock_block");

	ClassDB::bind_method(D_METHOD("set_filler_block", "block"), &BiomePreset::set_filler_block);
	ClassDB::bind_method(D_METHOD("get_filler_block"), &BiomePreset::get_filler_block);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "filler_block"), "set_filler_block", "get_filler_block");
}

BiomePreset::BiomePreset() {
	biome_name = "Default";
}

BiomePreset::~BiomePreset() {
}

void BiomePreset::set_biome_name(const String &p_name) {
	biome_name = p_name;
}

String BiomePreset::get_biome_name() const {
	return biome_name;
}

void BiomePreset::set_min_height(float p_height) {
	min_height = p_height;
}

float BiomePreset::get_min_height() const {
	return min_height;
}

void BiomePreset::set_max_height(float p_height) {
	max_height = p_height;
}

float BiomePreset::get_max_height() const {
	return max_height;
}

void BiomePreset::set_min_temperature(float p_temp) {
	min_temperature = p_temp;
}

float BiomePreset::get_min_temperature() const {
	return min_temperature;
}

void BiomePreset::set_max_temperature(float p_temp) {
	max_temperature = p_temp;
}

float BiomePreset::get_max_temperature() const {
	return max_temperature;
}

void BiomePreset::set_min_humidity(float p_humidity) {
	min_humidity = p_humidity;
}

float BiomePreset::get_min_humidity() const {
	return min_humidity;
}

void BiomePreset::set_max_humidity(float p_humidity) {
	max_humidity = p_humidity;
}

float BiomePreset::get_max_humidity() const {
	return max_humidity;
}

void BiomePreset::set_surface_blocks(const TypedArray<int32_t> &p_blocks) {
	surface_blocks = p_blocks;
}

TypedArray<int32_t> BiomePreset::get_surface_blocks() const {
	return surface_blocks;
}

void BiomePreset::set_subsurface_blocks(const TypedArray<int32_t> &p_blocks) {
	subsurface_blocks = p_blocks;
}

TypedArray<int32_t> BiomePreset::get_subsurface_blocks() const {
	return subsurface_blocks;
}

void BiomePreset::set_depth(int p_depth) {
	depth = CLAMP(p_depth, 1, 16);
}

int BiomePreset::get_depth() const {
	return depth;
}

void BiomePreset::set_bedrock_block(int p_block) {
	bedrock_block = p_block;
}

int BiomePreset::get_bedrock_block() const {
	return bedrock_block;
}

void BiomePreset::set_filler_block(int p_block) {
	filler_block = p_block;
}

int BiomePreset::get_filler_block() const {
	return filler_block;
}

} // namespace voxel_engine
