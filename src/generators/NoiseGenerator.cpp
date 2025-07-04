/**************************************************************************/
/*  NoiseGenerator.cpp                                                    */
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

#include "NoiseGenerator.h"
#include "../core/voxel_constants.h"

// Godot includes
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/fast_noise_lite.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace voxel_engine
{

    void NoiseGenerator::_bind_methods()
    {
        ClassDB::bind_method(D_METHOD("get_noise_3d", "x", "y", "z"), &NoiseGenerator::get_noise_3d);
        ClassDB::bind_method(D_METHOD("get_noise_2d", "x", "z"), &NoiseGenerator::get_noise_2d);

        ClassDB::bind_method(D_METHOD("set_seed", "seed"), &NoiseGenerator::set_seed);
        ClassDB::bind_method(D_METHOD("get_seed"), &NoiseGenerator::get_seed);

        ClassDB::bind_method(D_METHOD("set_noise", "noise"), &NoiseGenerator::set_noise);
        ClassDB::bind_method(D_METHOD("get_noise"), &NoiseGenerator::get_noise);

        ClassDB::bind_method(D_METHOD("set_octaves", "octaves"), &NoiseGenerator::set_octaves);
        ClassDB::bind_method(D_METHOD("get_octaves"), &NoiseGenerator::get_octaves);

        ClassDB::bind_method(D_METHOD("set_period", "period"), &NoiseGenerator::set_period);
        ClassDB::bind_method(D_METHOD("get_period"), &NoiseGenerator::get_period);

        ClassDB::bind_method(D_METHOD("set_persistence", "persistence"), &NoiseGenerator::set_persistence);
        ClassDB::bind_method(D_METHOD("get_persistence"), &NoiseGenerator::get_persistence);

        ClassDB::bind_method(D_METHOD("set_lacunarity", "lacunarity"), &NoiseGenerator::set_lacunarity);
        ClassDB::bind_method(D_METHOD("get_lacunarity"), &NoiseGenerator::get_lacunarity);

        // Define properties
        ADD_PROPERTY(PropertyInfo(Variant::INT, "seed"), "set_seed", "get_seed");
        ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "noise", PROPERTY_HINT_RESOURCE_TYPE, "Noise"), "set_noise", "get_noise");
        ADD_PROPERTY(PropertyInfo(Variant::INT, "octaves", PROPERTY_HINT_RANGE, "1,8,1"), "set_octaves", "get_octaves");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "period", PROPERTY_HINT_RANGE, "0.1,1000.0,0.1"), "set_period", "get_period");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "persistence", PROPERTY_HINT_RANGE, "0.0,1.0,0.01"), "set_persistence", "get_persistence");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "lacunarity", PROPERTY_HINT_RANGE, "1.0,4.0,0.01"), "set_lacunarity", "get_lacunarity");
    }

    NoiseGenerator::NoiseGenerator() : seed(12345),
                                       octaves(4),
                                       period(50.0f),
                                       persistence(0.5f),
                                       lacunarity(2.0f)
    {
        // Create default FastNoiseLite instance
        Ref<FastNoiseLite> fnoise;
        fnoise.instantiate();
        fnoise->set_noise_type(FastNoiseLite::TYPE_PERLIN);
        fnoise->set_seed(seed);
        fnoise->set_fractal_octaves(octaves);
        fnoise->set_frequency(1.0f / period);
        fnoise->set_fractal_gain(persistence);
        fnoise->set_fractal_lacunarity(lacunarity);
        fnoise->set_fractal_type(FastNoiseLite::FRACTAL_FBM);
        noise = fnoise;
    }

    NoiseGenerator::~NoiseGenerator()
    {
        // Destructor implementation
    }

    float NoiseGenerator::get_noise_3d(float x, float y, float z) const
    {
        if (noise.is_valid())
        {
            return noise->get_noise_3d(x, y, z);
        }
        return 0.0f;
    }

    float NoiseGenerator::get_noise_2d(float x, float z) const
    {
        if (noise.is_valid())
        {
            return noise->get_noise_2d(x, z);
        }
        return 0.0f;
    }

    void NoiseGenerator::set_seed(int p_seed)
    {
        seed = p_seed;

        if (noise.is_valid())
        {
            // Try to set seed on the noise object if it supports it
            Ref<FastNoiseLite> fnoise = dynamic_cast<FastNoiseLite *>(noise.ptr());
            if (fnoise.is_valid())
            {
                fnoise->set_seed(seed);
            }
        }
    }

    int NoiseGenerator::get_seed() const
    {
        return seed;
    }

    void NoiseGenerator::set_noise(Ref<Noise> p_noise)
    {
        noise = p_noise;

        // Set properties on the new noise object if possible
        Ref<FastNoiseLite> fnoise = dynamic_cast<FastNoiseLite *>(noise.ptr());
        if (fnoise.is_valid())
        {
            fnoise->set_seed(seed);
            fnoise->set_fractal_octaves(octaves);
            fnoise->set_frequency(1.0f / period);
            fnoise->set_fractal_gain(persistence);
            fnoise->set_fractal_lacunarity(lacunarity);
        }
    }

    Ref<Noise> NoiseGenerator::get_noise() const
    {
        return noise;
    }

    void NoiseGenerator::set_octaves(int p_octaves)
    {
        octaves = CLAMP(p_octaves, 1, 8);

        Ref<FastNoiseLite> fnoise = dynamic_cast<FastNoiseLite *>(noise.ptr());
        if (fnoise.is_valid())
        {
            fnoise->set_fractal_octaves(octaves);
        }
    }

    int NoiseGenerator::get_octaves() const
    {
        return octaves;
    }

    void NoiseGenerator::set_period(float p_period)
    {
        period = MAX(p_period, 0.1f);

        Ref<FastNoiseLite> fnoise = dynamic_cast<FastNoiseLite *>(noise.ptr());
        if (fnoise.is_valid())
        {
            fnoise->set_frequency(1.0f / period);
        }
    }

    float NoiseGenerator::get_period() const
    {
        return period;
    }

    void NoiseGenerator::set_persistence(float p_persistence)
    {
        persistence = CLAMP(p_persistence, 0.0f, 1.0f);

        Ref<FastNoiseLite> fnoise = dynamic_cast<FastNoiseLite *>(noise.ptr());
        if (fnoise.is_valid())
        {
            fnoise->set_fractal_gain(persistence);
        }
    }

    float NoiseGenerator::get_persistence() const
    {
        return persistence;
    }

    void NoiseGenerator::set_lacunarity(float p_lacunarity)
    {
        lacunarity = CLAMP(p_lacunarity, 1.0f, 4.0f);

        Ref<FastNoiseLite> fnoise = dynamic_cast<FastNoiseLite *>(noise.ptr());
        if (fnoise.is_valid())
        {
            fnoise->set_fractal_lacunarity(lacunarity);
        }
    }

    float NoiseGenerator::get_lacunarity() const
    {
        return lacunarity;
    }

} // namespace voxel_engine