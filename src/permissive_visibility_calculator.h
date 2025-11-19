#pragma once

#include "godot_cpp/classes/ref_counted.hpp"
#include "godot_cpp/templates/list.hpp"
#include <climits>

using namespace godot;

// From Adam Mil at http://www.adammil.net/blog/v125_roguelike_vision_algorithms.html#permissivecode

class PermissiveVisibilityCalculator : public RefCounted {
	GDCLASS(PermissiveVisibilityCalculator, RefCounted)

protected:
	static void _bind_methods();

public:
	/// Data structures

	struct Offset {
		friend class PermissiveVisibilityCalculator;

	public:
		Offset(int x, int y);
		Offset() = default;
		short x, y;
	};

	class Bump final {
		friend class PermissiveVisibilityCalculator;

	public:
		Bump(Bump *parent, Offset location);
		Bump() = default;
		Bump *parent;
		Offset location;
	};

	struct Line {
		friend class PermissiveVisibilityCalculator;

	public:
		Line(Offset near, Offset far);
		Line() = default;

		Offset near, far;

		bool is_below(Offset point);
		bool is_below_or_contains(Offset point);

		bool is_above(Offset point);
		bool is_above_or_contains(Offset point);

		bool does_contain(Offset point);

		// negative if the line is above the point.
		// positive if the line is below the point.
		// 0 if the line is on the point.
		int relative_slope(Offset point);
	};

	struct Field {
		friend class PermissiveVisibilityCalculator;

	public:
		Bump *steepBump, *shallowBump;
		Line steep, shallow;
	};

	/// Members

	Callable BlocksLight;
	//  readonly Func<int, int, int> GetDistance;
	Callable SetVisible;

	Offset source, quadrant;
	//  int rangeLimit;

	/// Methods

	// NOTE: Func and Action are C# constructs. I think the best approach is to convert these to Callables.
	// This comes with a downside: Callables can't be templated with typed arguments.
	// I'll look into alternatives, but I think this is a necessary sacrifice in this case.

	/// <param name="blocks_light">A function that accepts the X and Y coordinates of a tile and determines
	/// whether the given tile blocks the passage of light.
	/// </param>
	/// <param name="set_visible">A function that sets a tile to be visible, given its X and Y coordinates.</param>
	/// <param name="getDistance">A function that takes the X and Y coordinate of a point where X >= 0,
	/// Y >= 0, and X >= Y, and returns the distance from the point to the origin (0,0).
	/// </param>
	PermissiveVisibilityCalculator(Callable blocksLight, Callable setVisible); //,
	//								Func<int,int,int> getDistance);

	void compute(Vector2 origin); // , int rangeLimit);

	void compute_quadrant();

	bool act_is_blocked(Offset pos);

	List<Field>::Element *visit_square(Offset dest, List<Field>::Element *currentField, List<Field> activeFields);

	static void add_shallow_bump(Offset point, List<Field>::Element *currentField);

	static void add_steep_bump(Offset point, List<Field>::Element *currentField);

	static List<Field>::Element *check_field(List<Field>::Element *currentField, List<Field> activeFields);

	PermissiveVisibilityCalculator() = default;
	~PermissiveVisibilityCalculator() override = default;
};