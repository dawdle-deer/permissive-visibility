#pragma once

#include "godot_cpp/classes/ref_counted.hpp"
#include "godot_cpp/templates/list.hpp"
#include <climits>

using namespace godot;

// From Adam Mil at http://www.adammil.net/blog/v125_roguelike_vision_algorithms.html#permissivecode

class PermissiveVisibilityCalculatorGDExt : public RefCounted {
	GDCLASS(PermissiveVisibilityCalculatorGDExt, RefCounted)

protected:
	static void _bind_methods();

public:
	/// Data structures

	struct Line {
		friend class PermissiveVisibilityCalculatorGDExt;

	public:
		Vector2i near, far;

		inline bool is_below(Vector2i point);
		inline bool is_below_or_contains(Vector2i point);

		inline bool is_above(Vector2i point);
		inline bool is_above_or_contains(Vector2i point);

		inline bool does_contain(Vector2i point);

		// negative if the line is above the point.
		// positive if the line is below the point.
		// 0 if the line is on the point.
		inline int relative_slope(Vector2i point);
	};

	struct Field {
		friend class PermissiveVisibilityCalculatorGDExt;

	public:
		Line steep, shallow;
		List<Vector2i> steepBumps;
		List<Vector2i> shallowBumps;
	};

	/// Members

	/// @brief A function that accepts the X and Y coordinates of a tile and determines
	/// whether the given tile blocks the passage of light.
	Callable BlocksLight;
	/// @brief A function that sets a tile to be visible, given its X and Y coordinates.
	Callable SetVisible;
	/// @brief A function that takes the X and Y coordinate of a point where X >= 0,
	/// Y >= 0, and X >= Y, and returns the distance from the point to the origin (0,0).
	//  Callable GetDistance;

	Vector2i source = Vector2i(0, 0), quadrant = Vector2i(0, 0);
	//  int rangeLimit;

	/// Methods

	void compute(Vector2i origin); // , int rangeLimit);

	void compute_quadrant();

	bool act_is_blocked(Vector2i pos);

	List<Field>::Element *visit_square(Vector2i dest, List<Field>::Element *currentField, List<Field> *activeFields);

	static void add_shallow_bump(Vector2i point, List<Field>::Element *currentField);

	static void add_steep_bump(Vector2i point, List<Field>::Element *currentField);

	static List<Field>::Element *check_field(List<Field>::Element *currentField, List<Field> *activeFields);

	PermissiveVisibilityCalculatorGDExt() = default;
	~PermissiveVisibilityCalculatorGDExt() override = default;
};