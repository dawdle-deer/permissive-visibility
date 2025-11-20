#pragma once

#include "godot_cpp/classes/ref_counted.hpp"
#include "godot_cpp/templates/list.hpp"
#include <climits>

#include "permissive_visibility_data.h"

using namespace godot;

const short SHRT_ONE = 1;

// From Adam Mil at http://www.adammil.net/blog/v125_roguelike_vision_algorithms.html#permissivecode

class PermissiveVisibilityCalculatorGDExt : public RefCounted {
	GDCLASS(PermissiveVisibilityCalculatorGDExt, RefCounted)

protected:
	static void _bind_methods();

public:
	/// Data structures

	struct Offset {
		friend class PermissiveVisibilityCalculatorGDExt;

	public:
		short x, y;

		Offset() = default;
		Offset(short x, short y);
	};

	struct Bump {
		friend class PermissiveVisibilityCalculatorGDExt;

	public:
		Bump *parent;
		Offset location;
	};

	struct Line {
		friend class PermissiveVisibilityCalculatorGDExt;

	public:
		Offset near, far;

		inline bool is_below(Offset point);
		inline bool is_below_or_contains(Offset point);

		inline bool is_above(Offset point);
		inline bool is_above_or_contains(Offset point);

		inline bool does_contain(Offset point);

		// negative if the line is above the point.
		// positive if the line is below the point.
		// 0 if the line is on the point.
		inline int relative_slope(Offset point);
	};

	struct Field {
		friend class PermissiveVisibilityCalculatorGDExt;

	public:
		Line steep, shallow;
		Bump *steepBump = nullptr, *shallowBump = nullptr;

		void delete_bumps();
	};

	/// Members

	PermissiveVisibilityDataGDExt *data_reference;
	/// @brief A function that accepts the X and Y coordinates of a tile and determines
	/// whether the given tile blocks the passage of light.
	//bool (PermissiveVisibilityCalculatorGDExt::*BlocksLight)(int, int);
	/// @brief A function that sets a tile to be visible, given its X and Y coordinates.
	//void (PermissiveVisibilityCalculatorGDExt::*SetVisible)(int, int);
	/// @brief A function that takes the X and Y coordinate of a point where X >= 0,
	/// Y >= 0, and X >= Y, and returns the distance from the point to the origin (0,0).
	//  bool (*GetDistance)(int, int);

	Offset source = Offset(0, 0), quadrant = Offset(0, 0);
	//  int rangeLimit;

	/// Methods

	void compute(const Vector2i origin); // , int rangeLimit);

	void compute_quadrant();

	bool act_is_blocked(const Offset pos) const;

	List<Field>::Element *visit_square(const Offset dest, List<Field>::Element *currentField, List<Field> *activeFields);

	static void add_shallow_bump(const Offset point, List<Field>::Element *currentField);

	static void add_steep_bump(const Offset point, List<Field>::Element *currentField);

	static List<Field>::Element *check_field(List<Field>::Element *currentField, List<Field> *activeFields);

	PermissiveVisibilityCalculatorGDExt() = default;
	~PermissiveVisibilityCalculatorGDExt() override = default;
};