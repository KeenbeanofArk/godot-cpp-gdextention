/**************************************************************************/
/*  Voxel.h                                                               */
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

#ifndef VOXEL_H
#define VOXEL_H

// Godot includes
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/vector3.hpp>

using namespace godot;

namespace voxel_engine {

// Move the enum outside the class to make it accessible to Godot's type system
enum VoxelType {
	AIR,
	DIRT,
	GRASS,
	STONE,
	WATER,
	SAND,
	LAVA,
	GOLD,
	DIAMOND,
	IRON,
	COAL
	// Add more voxel types as needed
};

class Voxel : public RefCounted {
	GDCLASS(Voxel, RefCounted);

protected:
	static void _bind_methods();

public:
	Voxel();
	~Voxel();

	Vector3 get_position() const;
	void set_position(const Vector3 &position);

	int get_type() const;
	void set_type(int type);

	bool is_solid() const;

	void set_size(int size);
	int get_size() const;

private:
	Vector3 position;
	int type;
	int size;
};

} //namespace voxel_engine

// Register the VoxelType enum with Godot's type system
VARIANT_ENUM_CAST(voxel_engine::VoxelType);

#endif // VOXEL_H