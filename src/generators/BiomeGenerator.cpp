/**************************************************************************/
/*  BiomeGenerator.cpp                                                    */
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

#include "generators/BiomeGenerator.h"
#include "core/voxel.h"
#include "core/voxel_constants.h"
#include "generators/NoiseGenerator.h"

// Godot includes
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace voxel_engine {

// Position-seeded hash function for deterministic generation
uint32_t BiomeGenerator::hash_position(int x, int y, int z, int seed) {
	// FNV-1a hash variant
	uint32_t hash = 2166136261u;
	hash ^= static_cast<uint32_t>(x);
	hash *= 16777619u;
	hash ^= static_cast<uint32_t>(y);
	hash *= 16777619u;
	hash ^= static_cast<uint32_t>(z);
	hash *= 16777619u;
	hash ^= static_cast<uint32_t>(seed);
	hash *= 16777619u;

	// Additional mixing
	hash ^= hash >> 16;
	hash *= 0x85ebca6bu;
	hash ^= hash >> 13;
	hash *= 0xc2b2ae35u;
	hash ^= hash >> 16;

	return hash;
}

float BiomeGenerator::hash_to_float(uint32_t hash) {
	return static_cast<float>(hash & 0x00FFFFFFu) / static_cast<float>(0x01000000u);
}

int BiomeGenerator::hash_to_index(uint32_t hash, int count) {
	if (count <= 0)
		return 0;
	return static_cast<int>(hash % static_cast<uint32_t>(count));
}

void BiomeGenerator::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_height_noise", "noise"), &BiomeGenerator::set_height_noise);
	ClassDB::bind_method(D_METHOD("get_height_noise"), &BiomeGenerator::get_height_noise);

	ClassDB::bind_method(D_METHOD("set_temperature_noise", "noise"), &BiomeGenerator::set_temperature_noise);
	ClassDB::bind_method(D_METHOD("get_temperature_noise"), &BiomeGenerator::get_temperature_noise);

	ClassDB::bind_method(D_METHOD("set_humidity_noise", "noise"), &BiomeGenerator::set_humidity_noise);
	ClassDB::bind_method(D_METHOD("get_humidity_noise"), &BiomeGenerator::get_humidity_noise);

	ClassDB::bind_method(D_METHOD("set_sea_level", "sea_level"), &BiomeGenerator::set_sea_level);
	ClassDB::bind_method(D_METHOD("get_sea_level"), &BiomeGenerator::get_sea_level);

	ClassDB::bind_method(D_METHOD("set_seed", "seed"), &BiomeGenerator::set_seed);
	ClassDB::bind_method(D_METHOD("get_seed"), &BiomeGenerator::get_seed);

	ClassDB::bind_method(D_METHOD("set_default_voxel", "voxel_id"), &BiomeGenerator::set_default_voxel);
	ClassDB::bind_method(D_METHOD("get_default_voxel"), &BiomeGenerator::get_default_voxel);

	ClassDB::bind_method(D_METHOD("set_blend_distance", "distance"), &BiomeGenerator::set_blend_distance);
	ClassDB::bind_method(D_METHOD("get_blend_distance"), &BiomeGenerator::get_blend_distance);

	ClassDB::bind_method(D_METHOD("add_biome", "name", "min_height", "max_height", "min_temperature", "max_temperature", "min_humidity", "max_humidity", "surface_blocks", "subsurface_blocks", "depth"), &BiomeGenerator::add_biome);
	ClassDB::bind_method(D_METHOD("add_biome_extended", "name", "min_height", "max_height", "min_temperature", "max_temperature", "min_humidity", "max_humidity", "surface_blocks", "subsurface_blocks", "depth", "bedrock_block", "filler_block"), &BiomeGenerator::add_biome_extended);
	ClassDB::bind_method(D_METHOD("add_biome_with_y_ranges", "name", "min_height", "max_height", "min_temperature", "max_temperature", "min_humidity", "max_humidity", "surface_blocks", "subsurface_layers", "bedrock_block", "filler_block"), &BiomeGenerator::add_biome_with_y_ranges);
	ClassDB::bind_method(D_METHOD("clear_biomes"), &BiomeGenerator::clear_biomes);
	ClassDB::bind_method(D_METHOD("get_biome_count"), &BiomeGenerator::get_biome_count);
	ClassDB::bind_method(D_METHOD("get_biome_data", "index"), &BiomeGenerator::get_biome_data);

	ClassDB::bind_method(D_METHOD("get_height_at", "x", "z"), &BiomeGenerator::get_height_at);
	ClassDB::bind_method(D_METHOD("get_voxel_at", "x", "y", "z"), &BiomeGenerator::get_voxel_at);
	ClassDB::bind_method(D_METHOD("get_biome_index_at", "x", "z"), &BiomeGenerator::get_biome_index_at);

	ClassDB::bind_method(D_METHOD("get_blended_biome_weights", "x", "z"), &BiomeGenerator::get_blended_biome_weights);
	ClassDB::bind_method(D_METHOD("get_blended_height_at", "x", "z"), &BiomeGenerator::get_blended_height_at);

	ClassDB::bind_method(D_METHOD("get_temperature_at", "x", "z"), &BiomeGenerator::get_temperature_at);
	ClassDB::bind_method(D_METHOD("get_humidity_at", "x", "z"), &BiomeGenerator::get_humidity_at);

	// Define properties
	ADD_GROUP("Noise", "");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "height_noise", PROPERTY_HINT_RESOURCE_TYPE, "NoiseGenerator"), "set_height_noise", "get_height_noise");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "temperature_noise", PROPERTY_HINT_RESOURCE_TYPE, "NoiseGenerator"), "set_temperature_noise", "get_temperature_noise");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "humidity_noise", PROPERTY_HINT_RESOURCE_TYPE, "NoiseGenerator"), "set_humidity_noise", "get_humidity_noise");

	ADD_GROUP("Generation", "");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "seed"), "set_seed", "get_seed");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "sea_level"), "set_sea_level", "get_sea_level");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "default_voxel"), "set_default_voxel", "get_default_voxel");

	ADD_GROUP("Blending", "blend_");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "blend_distance", PROPERTY_HINT_RANGE, "8,32,1"), "set_blend_distance", "get_blend_distance");
}

BiomeGenerator::BiomeGenerator() :
		sea_level(0.0f),
		default_voxel(0),
		seed(DEFAULT_WORLD_SEED),
		blend_distance(DEFAULT_BLEND_DISTANCE) {
}

void BiomeGenerator::ensure_noise_generators() {
	if (!height_noise.is_valid()) {
		Ref<NoiseGenerator> h_noise;
		h_noise.instantiate();
		h_noise->set_period(100.0f);
		h_noise->set_seed(seed);
		height_noise = h_noise;
	}

	if (!temperature_noise.is_valid()) {
		Ref<NoiseGenerator> temp_noise;
		temp_noise.instantiate();
		temp_noise->set_period(200.0f);
		temp_noise->set_seed(seed + 1000);
		temperature_noise = temp_noise;
	}

	if (!humidity_noise.is_valid()) {
		Ref<NoiseGenerator> humid_noise;
		humid_noise.instantiate();
		humid_noise->set_period(180.0f);
		humid_noise->set_seed(seed + 2000);
		humidity_noise = humid_noise;
	}
}

BiomeGenerator::~BiomeGenerator() {
}

// Noise setters and getters
void BiomeGenerator::set_height_noise(const Ref<NoiseGenerator> &p_noise) {
	height_noise = p_noise;
}

Ref<NoiseGenerator> BiomeGenerator::get_height_noise() const {
	return height_noise;
}

void BiomeGenerator::set_temperature_noise(const Ref<NoiseGenerator> &p_noise) {
	temperature_noise = p_noise;
}

Ref<NoiseGenerator> BiomeGenerator::get_temperature_noise() const {
	return temperature_noise;
}

void BiomeGenerator::set_humidity_noise(const Ref<NoiseGenerator> &p_noise) {
	humidity_noise = p_noise;
}

Ref<NoiseGenerator> BiomeGenerator::get_humidity_noise() const {
	return humidity_noise;
}

void BiomeGenerator::set_sea_level(float p_sea_level) {
	sea_level = p_sea_level;
}

float BiomeGenerator::get_sea_level() const {
	return sea_level;
}

void BiomeGenerator::set_seed(int p_seed) {
	seed = p_seed;
	// Update noise generators with new seed if they exist
	if (height_noise.is_valid()) {
		height_noise->set_seed(seed);
	}
	if (temperature_noise.is_valid()) {
		temperature_noise->set_seed(seed + 1000);
	}
	if (humidity_noise.is_valid()) {
		humidity_noise->set_seed(seed + 2000);
	}
}

int BiomeGenerator::get_seed() const {
	return seed;
}

void BiomeGenerator::set_default_voxel(int p_voxel_id) {
	default_voxel = p_voxel_id;
}

int BiomeGenerator::get_default_voxel() const {
	return default_voxel;
}

void BiomeGenerator::set_blend_distance(int p_distance) {
	blend_distance = Math::clamp(p_distance, MIN_BLEND_DISTANCE, MAX_BLEND_DISTANCE);
}

int BiomeGenerator::get_blend_distance() const {
	return blend_distance;
}

void BiomeGenerator::add_biome(const String &name,
		float min_height, float max_height,
		float min_temperature, float max_temperature,
		float min_humidity, float max_humidity,
		const TypedArray<int> &surface_blocks,
		const TypedArray<int> &subsurface_blocks,
		int depth) {
	add_biome_extended(name, min_height, max_height,
			min_temperature, max_temperature,
			min_humidity, max_humidity,
			surface_blocks, subsurface_blocks, depth,
			VoxelType::STONE, VoxelType::STONE);
}

void BiomeGenerator::add_biome_extended(const String &name,
		float min_height, float max_height,
		float min_temperature, float max_temperature,
		float min_humidity, float max_humidity,
		const TypedArray<int32_t> &surface_blocks,
		const TypedArray<int32_t> &subsurface_blocks,
		int depth,
		int bedrock_block,
		int filler_block) {
	BiomeData biome;
	biome.name = name;
	biome.min_height = min_height;
	biome.max_height = max_height;
	biome.min_temperature = min_temperature;
	biome.max_temperature = max_temperature;
	biome.min_humidity = min_humidity;
	biome.max_humidity = max_humidity;
	biome.surface_blocks = surface_blocks;
	biome.subsurface_blocks = subsurface_blocks;
	biome.depth = depth;
	biome.bedrock_block = bedrock_block;
	biome.filler_block = filler_block;

	biomes.push_back(biome);
}

void BiomeGenerator::add_biome_with_y_ranges(const String &name,
		float min_height, float max_height,
		float min_temperature, float max_temperature,
		float min_humidity, float max_humidity,
		const TypedArray<int32_t> &surface_blocks,
		const Array &subsurface_layers_array,
		int bedrock_block,
		int filler_block) {
	BiomeData biome;
	biome.name = name;
	biome.min_height = min_height;
	biome.max_height = max_height;
	biome.min_temperature = min_temperature;
	biome.max_temperature = max_temperature;
	biome.min_humidity = min_humidity;
	biome.max_humidity = max_humidity;
	biome.surface_blocks = surface_blocks;
	biome.bedrock_block = bedrock_block;
	biome.filler_block = filler_block;

	// Convert Array of Dictionaries to Vector<SubsurfaceLayer>
	for (int i = 0; i < subsurface_layers_array.size(); ++i) {
		Variant v = subsurface_layers_array[i];
		if (v.get_type() == Variant::DICTIONARY) {
			Dictionary dict = v;
			SubsurfaceLayer layer;
			if (dict.has("block_type")) {
				layer.block_type = static_cast<int>(dict["block_type"]);
			}
			if (dict.has("y_min")) {
				layer.y_min = static_cast<int>(dict["y_min"]);
			}
			if (dict.has("y_max")) {
				layer.y_max = static_cast<int>(dict["y_max"]);
			}
			if (dict.has("density")) {
				layer.density = static_cast<float>(dict["density"]);
			}
			biome.subsurface_layers.push_back(layer);
		}
	}

	// Note: depth field left uninitialized (DEPRECATED - use subsurface_layers instead)
	// Note: subsurface_blocks left uninitialized (DEPRECATED - use subsurface_layers instead)

	biomes.push_back(biome);
}

void BiomeGenerator::clear_biomes() {
	biomes.clear();
}

int BiomeGenerator::get_biome_count() const {
	return biomes.size();
}

Dictionary BiomeGenerator::get_biome_data(int index) const {
	Dictionary result;
	if (index >= 0 && index < biomes.size()) {
		const BiomeData &biome = biomes[index];
		result["name"] = biome.name;
		result["min_height"] = biome.min_height;
		result["max_height"] = biome.max_height;
		result["min_temperature"] = biome.min_temperature;
		result["max_temperature"] = biome.max_temperature;
		result["min_humidity"] = biome.min_humidity;
		result["max_humidity"] = biome.max_humidity;
		result["surface_blocks"] = biome.surface_blocks;
		result["subsurface_blocks"] = biome.subsurface_blocks;
		result["depth"] = biome.depth;
		result["bedrock_block"] = biome.bedrock_block;
		result["filler_block"] = biome.filler_block;
	}
	return result;
}

int BiomeGenerator::select_block_deterministic(int x, int y, int z, const TypedArray<int32_t> &blocks) const {
	if (blocks.size() == 0) {
		return VoxelType::AIR;
	}
	if (blocks.size() == 1) {
		return blocks[0];
	}

	uint32_t hash = hash_position(x, y, z, seed);
	int index = hash_to_index(hash, blocks.size());
	return blocks[index];
}

float BiomeGenerator::get_height_at(float x, float z) const {
	const_cast<BiomeGenerator *>(this)->ensure_noise_generators();

	if (biomes.size() == 0) {
		return 0.0f;
	}

	float noise_value = height_noise->get_noise_2d(x, z);
	float height = (noise_value + 1.0f) * 0.5f * 100.0f; // Scale to 0-100 range

	return height > sea_level ? height : sea_level;
}

float BiomeGenerator::get_blended_height_at(float x, float z) const {
	const_cast<BiomeGenerator *>(this)->ensure_noise_generators();

	if (biomes.size() == 0) {
		return 0.0f;
	}

	// Get biome weights for blending
	Array weights = get_blended_biome_weights(x, z);

	// Fallback to default height when no biome weights are available
	if (weights.size() == 0) {
		return get_height_at(x, z);
	}

	// Sample normalized height noise (0..1) to drive per-biome interpolation
	float n = 0.5f;
	if (height_noise.is_valid()) {
		float nv = height_noise->get_noise_2d(x, z);
		n = (nv + 1.0f) * 0.5f;
	}

	// Blend per-biome heights using each biome's min/max height range.
	// BiomeData stores heights in normalized form (0..1), while
	// get_height_at() uses a 0..100 range. To keep compatibility we
	// map biome min/max -> 0..100 here.
	float blended = 0.0f;
	float total_weight = 0.0f;

	for (int i = 0; i < weights.size(); ++i) {
		Dictionary bw = weights[i];
		int biome_index = static_cast<int>(bw["biome_index"]);
		float weight = static_cast<float>(bw["weight"]);
		if (biome_index < 0 || biome_index >= biomes.size())
			continue;

		const BiomeData &bd = biomes[biome_index];

		// Map biome min/max to height range.
		// Supports two authoring modes:
		// 1) Normalized 0..1 (or -1..1) -> scaled to 0..100
		// 2) Absolute 0..100 units -> used as-is
		float min_h = bd.min_height;
		float max_h = bd.max_height;
		if (max_h <= 1.0f && min_h >= -1.0f) {
			min_h *= 100.0f;
			max_h *= 100.0f;
		}
		float local_h = min_h + (max_h - min_h) * n;

		blended += local_h * weight;
		total_weight += weight;
	}

	if (total_weight > 0.0f) {
		blended /= total_weight;
	} else {
		// Fallback: use procedural height
		blended = get_height_at(x, z);
	}

	return blended > sea_level ? blended : sea_level;
}

Array BiomeGenerator::get_blended_biome_weights(float x, float z) const {
	Array result;

	if (biomes.size() == 0) {
		return result;
	}

	// Get the primary biome at this position
	int primary_biome = get_biome_index_at(x, z);
	if (primary_biome < 0) {
		return result;
	}

	// Sample surrounding points to detect biome boundaries
	float sample_dist = static_cast<float>(blend_distance);
	HashMap<int, float> biome_contributions;

	// Sample in a grid pattern around the position
	const int samples = 5;
	float total_weight = 0.0f;

	for (int sx = -samples / 2; sx <= samples / 2; sx++) {
		for (int sz = -samples / 2; sz <= samples / 2; sz++) {
			float sample_x = x + sx * (sample_dist / samples);
			float sample_z = z + sz * (sample_dist / samples);

			int biome_at_sample = get_biome_index_at(sample_x, sample_z);
			if (biome_at_sample < 0)
				continue;

			// Weight based on distance from center (closer = higher weight)
			float dist = Math::sqrt(static_cast<float>(sx * sx + sz * sz));
			float weight = 1.0f / (1.0f + dist);

			if (biome_contributions.has(biome_at_sample)) {
				biome_contributions[biome_at_sample] += weight;
			} else {
				biome_contributions[biome_at_sample] = weight;
			}
			total_weight += weight;
		}
	}

	// Normalize weights and create result array
	for (const KeyValue<int, float> &kv : biome_contributions) {
		Dictionary biome_weight;
		biome_weight["biome_index"] = kv.key;
		biome_weight["weight"] = kv.value / total_weight;
		result.push_back(biome_weight);
	}

	return result;
}

int BiomeGenerator::select_voxel_with_blending(int x, int y, int z, const Array &biome_weights, bool is_surface) const {
	if (biome_weights.size() == 0 || biomes.size() == 0) {
		return VoxelType::AIR;
	}

	// Use position hash to deterministically select which biome's block to use
	uint32_t hash = hash_position(x, y, z, seed + 54321);
	float selection = hash_to_float(hash);

	// Accumulate weights to select biome
	float accumulated = 0.0f;
	int selected_biome = -1;

	for (int i = 0; i < biome_weights.size(); i++) {
		Dictionary bw = biome_weights[i];
		float weight = bw["weight"];
		accumulated += weight;

		if (selection <= accumulated) {
			selected_biome = bw["biome_index"];
			break;
		}
	}

	if (selected_biome < 0 || selected_biome >= biomes.size()) {
		// Fallback to first biome in weights
		Dictionary first = biome_weights[0];
		selected_biome = first["biome_index"];
	}

	const BiomeData &biome = biomes[selected_biome];

	if (is_surface) {
		return select_block_deterministic(x, y, z, biome.surface_blocks);
	} else {
		return select_block_deterministic(x, y, z, biome.subsurface_blocks);
	}
}

Ref<Voxel> BiomeGenerator::get_voxel_at(int x, int y, int z) const {
	Ref<Voxel> voxel;
	voxel.instantiate();

	if (biomes.size() == 0) {
		voxel->set_type(VoxelType::AIR);
		return voxel;
	}

	// Use blended height so voxel layer selection matches terrain surface used by the generator.
	float height = get_blended_height_at(static_cast<float>(x), static_cast<float>(z));

	// Above ground
	if (y > height) {
		if (y <= sea_level) {
			voxel->set_type(VoxelType::WATER);
		} else {
			voxel->set_type(VoxelType::AIR);
		}
		return voxel;
	}

	// Get blended biome weights for this position
	Array biome_weights = get_blended_biome_weights(static_cast<float>(x), static_cast<float>(z));

	// Get primary biome for layer depths
	int primary_biome_index = get_biome_index_at(x, z);
	if (primary_biome_index < 0) {
		voxel->set_type(VoxelType::AIR);
		return voxel;
	}

	const BiomeData &primary_biome = biomes[primary_biome_index];

	// ====== P2: Y-Range Based Layer Selection (NEW SYSTEM) ======
	// First, try Y-range based subsurface layers (if defined)
	if (primary_biome.subsurface_layers.size() > 0) {
		// Iterate through Y-range layers in priority order (first match wins)
		for (const auto &layer : primary_biome.subsurface_layers) {
			if (y >= layer.y_min && y <= layer.y_max) {
				// Found matching Y-range layer
				// Use the block_type with density consideration
				int block_type = layer.block_type;
				if (block_type != 0) {
					// Apply density: if density < 1.0, sometimes use default voxel based on hash
					if (layer.density >= 1.0f) {
						voxel->set_type(block_type);
					} else {
						// Probabilistic density: use hash to determine if block appears
						uint32_t hash = hash_position(x, y, z, seed);
						float rand_val = hash_to_float(hash);
						if (rand_val < layer.density) {
							voxel->set_type(block_type);
						} else {
							// Fallback to blended subsurface based on density miss
							int voxel_type = select_voxel_with_blending(x, y, z, biome_weights, false);
							if (voxel_type == 0 && primary_biome.subsurface_blocks.size() > 0) {
								voxel_type = select_block_deterministic(x, y, z, primary_biome.subsurface_blocks);
							}
							voxel->set_type(voxel_type);
						}
					}
					voxel->set_position(Vector3(x, y, z));
					return voxel;
				}
				break; // Match found, don't check further ranges even if block_type is 0
			}
		}
	}

	// ====== Depth-Based Layer Selection (FALLBACK - OLD SYSTEM) ======
	// Use traditional depth-based logic if:
	// 1. No Y-range layers are defined, OR
	// 2. Y is outside all Y-range layers (need to fill gaps)

	// Determine layer based on depth
	int depth_from_surface = static_cast<int>(height) - y;

	if (depth_from_surface < primary_biome.depth) {
		// Surface layer - use blended selection
		int voxel_type = select_voxel_with_blending(x, y, z, biome_weights, true);
		if (voxel_type == 0 && primary_biome.surface_blocks.size() > 0) {
			voxel_type = select_block_deterministic(x, y, z, primary_biome.surface_blocks);
		}
		voxel->set_type(voxel_type);
	} else if (y <= 5) {
		// Bedrock layer (bottom 5 blocks)
		voxel->set_type(primary_biome.bedrock_block);
	} else if (depth_from_surface < primary_biome.depth * 4) {
		// Subsurface layer - use blended selection
		int voxel_type = select_voxel_with_blending(x, y, z, biome_weights, false);
		if (voxel_type == 0 && primary_biome.subsurface_blocks.size() > 0) {
			voxel_type = select_block_deterministic(x, y, z, primary_biome.subsurface_blocks);
		}
		voxel->set_type(voxel_type);
	} else {
		// Deep filler layer
		voxel->set_type(primary_biome.filler_block);
	}

	voxel->set_position(Vector3(x, y, z));
	return voxel;
}

int BiomeGenerator::get_biome_index_at(float x, float z) const {
	if (biomes.size() == 0) {
		return -1;
	}

	float temp = get_temperature_at(x, z);
	float humid = get_humidity_at(x, z);
	float height = get_height_at(x, z);

	int closest_biome_index = -1;
	float closest_match = 9999.0f;

	for (int i = 0; i < biomes.size(); i++) {
		const BiomeData &biome = biomes[i];

		if (height < biome.min_height || height > biome.max_height) {
			continue;
		}

		if (temp < biome.min_temperature || temp > biome.max_temperature ||
				humid < biome.min_humidity || humid > biome.max_humidity) {
			continue;
		}

		float temp_match = Math::abs((biome.min_temperature + biome.max_temperature) * 0.5f - temp);
		float humid_match = Math::abs((biome.min_humidity + biome.max_humidity) * 0.5f - humid);
		float total_match = temp_match + humid_match;

		if (total_match < closest_match) {
			closest_match = total_match;
			closest_biome_index = i;
		}
	}

	if (closest_biome_index == -1 && biomes.size() > 0) {
		return 0;
	}

	return closest_biome_index;
}

float BiomeGenerator::get_temperature_at(float x, float z) const {
	const_cast<BiomeGenerator *>(this)->ensure_noise_generators();

	float noise_value = temperature_noise->get_noise_2d(x, z);
	return (noise_value + 1.0f) * 0.5f;
}

float BiomeGenerator::get_humidity_at(float x, float z) const {
	const_cast<BiomeGenerator *>(this)->ensure_noise_generators();

	float noise_value = humidity_noise->get_noise_2d(x, z);
	return (noise_value + 1.0f) * 0.5f;
}

} // namespace voxel_engine
