/**************************************************************************/
/*  BiomeGenerator.h                                                      */
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

#ifndef BIOME_GENERATOR_H
#define BIOME_GENERATOR_H

#include "core/voxel.h"
#include "NoiseGenerator.h"

// Godot includes
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/templates/vector.hpp>

using namespace godot;

namespace voxel_engine
{

    struct BiomeData
    {
        String name;
        float min_height;
        float max_height;
        float min_temperature;
        float max_temperature;
        float min_humidity;
        float max_humidity;
        TypedArray<int32_t> surface_blocks;
        TypedArray<int32_t> subsurface_blocks;
        int depth;

        BiomeData() : min_height(-1.0f),
                      max_height(1.0f),
                      min_temperature(-1.0f),
                      max_temperature(1.0f),
                      min_humidity(-1.0f),
                      max_humidity(1.0f),
                      depth(4)
        {
        }
    };

    class BiomeGenerator : public Resource
    {
        GDCLASS(BiomeGenerator, Resource);

    private:
        Ref<NoiseGenerator> height_noise;
        Ref<NoiseGenerator> temperature_noise;
        Ref<NoiseGenerator> humidity_noise;

        Vector<BiomeData> biomes;
        float sea_level;
        int default_voxel;

    protected:
        static void _bind_methods();

    public:
        BiomeGenerator();
        ~BiomeGenerator();

        // Noise setters and getters
        void set_height_noise(const Ref<NoiseGenerator> &p_noise);
        Ref<NoiseGenerator> get_height_noise() const;

        void set_temperature_noise(const Ref<NoiseGenerator> &p_noise);
        Ref<NoiseGenerator> get_temperature_noise() const;

        void set_humidity_noise(const Ref<NoiseGenerator> &p_noise);
        Ref<NoiseGenerator> get_humidity_noise() const;

        // Sea level control
        void set_sea_level(float p_sea_level);
        float get_sea_level() const;

        // Default voxel for empty spaces
        void set_default_voxel(int p_voxel_id);
        int get_default_voxel() const;

        // Biome management
        void add_biome(const String &name,
                       float min_height, float max_height,
                       float min_temperature, float max_temperature,
                       float min_humidity, float max_humidity,
                       const TypedArray<int32_t> &surface_blocks,
                       const TypedArray<int32_t> &subsurface_blocks,
                       int depth);

        void clear_biomes();
        int get_biome_count() const;

        // Generation methods
        float get_height_at(float x, float z) const;
        Ref<Voxel> get_voxel_at(int x, int y, int z) const; // Changed return type to Ref<Voxel>
        int get_biome_index_at(float x, float z) const;

        // Utility functions
        float get_temperature_at(float x, float z) const;
        float get_humidity_at(float x, float z) const;
    };

} // namespace voxel_engine

#endif // BIOME_GENERATOR_H