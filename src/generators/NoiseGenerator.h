/**************************************************************************/
/*  NoiseGenerator.h                                                      */
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

#ifndef NOISE_GENERATOR_H
#define NOISE_GENERATOR_H

// Godot includes
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/noise.hpp>
#include <godot_cpp/variant/vector3.hpp>

using namespace godot;

namespace voxel_engine
{

    // NoiseGenerator class for generating terrain height and density values
    class NoiseGenerator : public RefCounted
    {
        GDCLASS(NoiseGenerator, RefCounted);

    protected:
        static void _bind_methods();

    public:
        NoiseGenerator();
        ~NoiseGenerator();

        // Noise generation methods
        float get_noise_3d(float x, float y, float z) const;
        float get_noise_2d(float x, float z) const;

        // Configuration methods
        void set_seed(int p_seed);
        int get_seed() const;

        void set_noise(Ref<Noise> p_noise);
        Ref<Noise> get_noise() const;

        void set_octaves(int p_octaves);
        int get_octaves() const;

        void set_period(float p_period);
        float get_period() const;

        void set_persistence(float p_persistence);
        float get_persistence() const;

        void set_lacunarity(float p_lacunarity);
        float get_lacunarity() const;

    private:
        int seed;
        Ref<Noise> noise;
        int octaves;
        float period;
        float persistence;
        float lacunarity;
    };

} // namespace voxel_engine

#endif // NOISE_GENERATOR_H