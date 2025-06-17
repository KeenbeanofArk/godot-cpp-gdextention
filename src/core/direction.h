// direction.h
#pragma once

#include <godot_cpp/variant/vector3i.hpp>

namespace voxel_engine {

class Direction {
public:
	enum Value : uint8_t {
		NEGATIVE_X = 0,
		POSITIVE_X = 1,
		NEGATIVE_Y = 2,
		POSITIVE_Y = 3,
		NEGATIVE_Z = 4,
		POSITIVE_Z = 5,

		// Additional alias values for convenience
		LEFT = NEGATIVE_X,
		RIGHT = POSITIVE_X,
		DOWN = NEGATIVE_Y,
		UP = POSITIVE_Y,
		BACK = NEGATIVE_Z,
		FRONT = POSITIVE_Z,

		// Count of primary directions (X+, X-, Y+, Y-, Z+, Z-)
		COUNT = 6
	};

	// Get the opposite direction
	static Value get_opposite(Value direction) {
		return Value(direction ^ 1);
	}

	// Get a unit vector representing the given direction
	static godot::Vector3i get_direction_vector(Value direction) {
		static const godot::Vector3i vectors[COUNT] = {
			godot::Vector3i(-1, 0, 0), // NEGATIVE_X
			godot::Vector3i(1, 0, 0), // POSITIVE_X
			godot::Vector3i(0, -1, 0), // NEGATIVE_Y
			godot::Vector3i(0, 1, 0), // POSITIVE_Y
			godot::Vector3i(0, 0, -1), // NEGATIVE_Z
			godot::Vector3i(0, 0, 1) // POSITIVE_Z
		};

		return vectors[direction];
	}

	// Get the direction value from a normalized direction vector
	static Value from_vector(const godot::Vector3i &vector) {
		if (vector.x < 0)
			return NEGATIVE_X;
		if (vector.x > 0)
			return POSITIVE_X;
		if (vector.y < 0)
			return NEGATIVE_Y;
		if (vector.y > 0)
			return POSITIVE_Y;
		if (vector.z < 0)
			return NEGATIVE_Z;
		if (vector.z > 0)
			return POSITIVE_Z;

		// Default if not a cardinal direction
		return POSITIVE_Y;
	}

	// Get the name of the direction as a string
	static const char *get_name(Value direction) {
		static const char *names[COUNT] = {
			"NEGATIVE_X",
			"POSITIVE_X",
			"NEGATIVE_Y",
			"POSITIVE_Y",
			"NEGATIVE_Z",
			"POSITIVE_Z"
		};

		return names[direction];
	}
};

enum Direction2D {
	NORTH = 0,
	SOUTH = 1,
	EAST = 2,
	WEST = 3,
	UP = 4,
	DOWN = 5
};

} // namespace voxel_engine
