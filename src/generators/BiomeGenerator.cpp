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

namespace voxel_engine
{
    void BiomeGenerator::_bind_methods()
    {
        ClassDB::bind_method(D_METHOD("set_height_noise", "noise"), &BiomeGenerator::set_height_noise);
        ClassDB::bind_method(D_METHOD("get_height_noise"), &BiomeGenerator::get_height_noise);

        ClassDB::bind_method(D_METHOD("set_temperature_noise", "noise"), &BiomeGenerator::set_temperature_noise);
        ClassDB::bind_method(D_METHOD("get_temperature_noise"), &BiomeGenerator::get_temperature_noise);

        ClassDB::bind_method(D_METHOD("set_humidity_noise", "noise"), &BiomeGenerator::set_humidity_noise);
        ClassDB::bind_method(D_METHOD("get_humidity_noise"), &BiomeGenerator::get_humidity_noise);

        ClassDB::bind_method(D_METHOD("set_sea_level", "sea_level"), &BiomeGenerator::set_sea_level);
        ClassDB::bind_method(D_METHOD("get_sea_level"), &BiomeGenerator::get_sea_level);

        ClassDB::bind_method(D_METHOD("set_default_voxel", "voxel_id"), &BiomeGenerator::set_default_voxel);
        ClassDB::bind_method(D_METHOD("get_default_voxel"), &BiomeGenerator::get_default_voxel);

        ClassDB::bind_method(D_METHOD("add_biome", "name", "min_height", "max_height", "min_temperature", "max_temperature", "min_humidity", "max_humidity", "surface_blocks", "subsurface_blocks", "depth"), &BiomeGenerator::add_biome);
        ClassDB::bind_method(D_METHOD("clear_biomes"), &BiomeGenerator::clear_biomes);
        ClassDB::bind_method(D_METHOD("get_biome_count"), &BiomeGenerator::get_biome_count);

        ClassDB::bind_method(D_METHOD("get_height_at", "x", "z"), &BiomeGenerator::get_height_at);
        ClassDB::bind_method(D_METHOD("get_voxel_at", "x", "y", "z"), &BiomeGenerator::get_voxel_at);
        ClassDB::bind_method(D_METHOD("get_biome_index_at", "x", "z"), &BiomeGenerator::get_biome_index_at);

        ClassDB::bind_method(D_METHOD("get_temperature_at", "x", "z"), &BiomeGenerator::get_temperature_at);
        ClassDB::bind_method(D_METHOD("get_humidity_at", "x", "z"), &BiomeGenerator::get_humidity_at);

        // Define properties
        ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "height_noise", PROPERTY_HINT_RESOURCE_TYPE, "NoiseGenerator"), "set_height_noise", "get_height_noise");
        ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "temperature_noise", PROPERTY_HINT_RESOURCE_TYPE, "NoiseGenerator"), "set_temperature_noise", "get_temperature_noise");
        ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "humidity_noise", PROPERTY_HINT_RESOURCE_TYPE, "NoiseGenerator"), "set_humidity_noise", "get_humidity_noise");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "sea_level"), "set_sea_level", "get_sea_level");
        ADD_PROPERTY(PropertyInfo(Variant::INT, "default_voxel"), "set_default_voxel", "get_default_voxel");
    }

    BiomeGenerator::BiomeGenerator() : sea_level(0.0f), default_voxel(0)
    {
        // Create default noise generators
        Ref<NoiseGenerator> h_noise;
        h_noise.instantiate();
        h_noise->set_period(100.0f);
        height_noise = h_noise;

        Ref<NoiseGenerator> temp_noise;
        temp_noise.instantiate();
        temp_noise->set_period(200.0f); // Large period for smooth transitions
        temperature_noise = temp_noise;

        Ref<NoiseGenerator> humid_noise;
        humid_noise.instantiate();
        humid_noise->set_period(180.0f); // Slightly different period to prevent alignment
        humidity_noise = humid_noise;
    }

    BiomeGenerator::~BiomeGenerator()
    {
        // Destructor implementation
    }

    // Noise setters and getters
    void BiomeGenerator::set_height_noise(const Ref<NoiseGenerator> &p_noise)
    {
        height_noise = p_noise;
    }

    Ref<NoiseGenerator> BiomeGenerator::get_height_noise() const
    {
        return height_noise;
    }

    void BiomeGenerator::set_temperature_noise(const Ref<NoiseGenerator> &p_noise)
    {
        temperature_noise = p_noise;
    }

    Ref<NoiseGenerator> BiomeGenerator::get_temperature_noise() const
    {
        return temperature_noise;
    }

    void BiomeGenerator::set_humidity_noise(const Ref<NoiseGenerator> &p_noise)
    {
        humidity_noise = p_noise;
    }

    Ref<NoiseGenerator> BiomeGenerator::get_humidity_noise() const
    {
        return humidity_noise;
    }

    // Sea level control
    void BiomeGenerator::set_sea_level(float p_sea_level)
    {
        sea_level = p_sea_level;
    }

    float BiomeGenerator::get_sea_level() const
    {
        return sea_level;
    }

    // Default voxel for empty spaces
    void BiomeGenerator::set_default_voxel(int p_voxel_id)
    {
        default_voxel = p_voxel_id;
    }

    int BiomeGenerator::get_default_voxel() const
    {
        return default_voxel;
    }

    // Biome management
    void BiomeGenerator::add_biome(const String &name,
                                   float min_height, float max_height,
                                   float min_temperature, float max_temperature,
                                   float min_humidity, float max_humidity,
                                   const TypedArray<int> &surface_blocks,
                                   const TypedArray<int> &subsurface_blocks,
                                   int depth)
    {
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

        biomes.push_back(biome);
    }

    void BiomeGenerator::clear_biomes()
    {
        biomes.clear();
    }

    int BiomeGenerator::get_biome_count() const
    {
        return biomes.size();
    }

    // Generation methods
    float BiomeGenerator::get_height_at(float x, float z) const
    {
        if (biomes.size() == 0 || !height_noise.is_valid())
        {
            return 0.0f;
        }

        // Get noise value for this position
        float noise_value = height_noise->get_noise_2d(x, z);

        // Map noise from [-1,1] to a range suitable for terrain
        float height = (noise_value + 1.0f) * 0.5f * 100.0f; // Scale to 0-100 range

        // Apply sea level
        return height > sea_level ? height : sea_level;
    }

    Ref<Voxel> BiomeGenerator::get_voxel_at(int x, int y, int z) const
    {
        Ref<Voxel> voxel;
        voxel.instantiate();

        if (biomes.size() == 0)
        {
            // Return default voxel (likely air)
            voxel->set_type(VoxelType::AIR); // Default to air
            return voxel;
        }

        float height = get_height_at(x, z);
        if (y > height)
        {
            // Above ground, return air or water based on sea level
            if (y <= sea_level)
            {
                voxel->set_type(VoxelType::WATER); // Water voxel
            }
            else
            {
                voxel->set_type(VoxelType::AIR); // Air voxel
            }
            return voxel;
        }

        // Get biome at this position
        int biome_index = get_biome_index_at(x, z);
        if (biome_index < 0)
        {
            voxel->set_type(VoxelType::AIR);
            return voxel;
        }

        const BiomeData &biome = biomes[biome_index];

        // Determine if we're at the surface or below
        if (y > height - biome.depth)
        {
            // Surface layer - select a random block from surface options
            if (biome.surface_blocks.size() > 0)
            {
                int index = Math::abs(int(UtilityFunctions::randf() * biome.surface_blocks.size())) % biome.surface_blocks.size();
                // Convert the integer voxel type to enum value (assuming the integers match the enum values)
                int voxel_type = biome.surface_blocks[index];
                // Clamp to valid range to prevent issues
                voxel_type = Math::clamp(voxel_type, 0, int(VoxelType::WATER));
                voxel->set_type(static_cast<VoxelType>(voxel_type));
            }
            else
            {
                voxel->set_type(VoxelType::DIRT); // Default to dirt if no surface blocks defined
            }
        }
        else
        {
            // Subsurface layer - select a random block from subsurface options
            if (biome.subsurface_blocks.size() > 0)
            {
                int index = Math::abs(int(UtilityFunctions::randf() * biome.subsurface_blocks.size())) % biome.subsurface_blocks.size();
                // Convert the integer voxel type to enum value
                int voxel_type = biome.subsurface_blocks[index];
                // Clamp to valid range to prevent issues
                voxel_type = Math::clamp(voxel_type, 0, int(VoxelType::WATER));
                voxel->set_type(static_cast<VoxelType>(voxel_type));
            }
            else
            {
                voxel->set_type(VoxelType::STONE); // Default to stone if no subsurface blocks defined
            }
        }

        // Set the position of the voxel
        voxel->set_position(Vector3(x, y, z));

        return voxel;
    }

    int BiomeGenerator::get_biome_index_at(float x, float z) const
    {
        if (biomes.size() == 0)
        {
            return -1;
        }

        // Get temperature and humidity at this position
        float temp = get_temperature_at(x, z);
        float humid = get_humidity_at(x, z);
        float height = get_height_at(x, z);

        // Find the most suitable biome based on height, temperature and humidity
        int closest_biome_index = -1;
        float closest_match = 9999.0f; // Large value to start

        for (int i = 0; i < biomes.size(); i++)
        {
            const BiomeData &biome = biomes[i];

            // Check if height is in range
            if (height < biome.min_height || height > biome.max_height)
            {
                continue;
            }

            // Check temperature and humidity match
            if (temp < biome.min_temperature || temp > biome.max_temperature ||
                humid < biome.min_humidity || humid > biome.max_humidity)
            {
                continue;
            }

            // Calculate how well this biome matches (lower is better)
            float temp_match = Math::abs((biome.min_temperature + biome.max_temperature) * 0.5f - temp);
            float humid_match = Math::abs((biome.min_humidity + biome.max_humidity) * 0.5f - humid);
            float total_match = temp_match + humid_match;

            if (total_match < closest_match)
            {
                closest_match = total_match;
                closest_biome_index = i;
            }
        }

        // If no suitable biome found, return the first one or -1
        if (closest_biome_index == -1 && biomes.size() > 0)
        {
            return 0;
        }

        return closest_biome_index;
    }

    // Utility functions
    float BiomeGenerator::get_temperature_at(float x, float z) const
    {
        if (!temperature_noise.is_valid())
        {
            return 0.5f; // Default temperature
        }

        // Map noise from [-1,1] to [0,1] for temperature
        float noise_value = temperature_noise->get_noise_2d(x, z);
        return (noise_value + 1.0f) * 0.5f;
    }

    float BiomeGenerator::get_humidity_at(float x, float z) const
    {
        if (!humidity_noise.is_valid())
        {
            return 0.5f; // Default humidity
        }

        // Map noise from [-1,1] to [0,1] for humidity
        float noise_value = humidity_noise->get_noise_2d(x, z);
        return (noise_value + 1.0f) * 0.5f;
    }

} // namespace voxel_engine