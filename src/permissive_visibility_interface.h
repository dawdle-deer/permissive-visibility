#pragma once

#include "godot_cpp/classes/ref_counted.hpp"

#include "permissive_visibility_calculator.h"

using namespace godot;

class PermissiveVisibilityInterface : public RefCounted {
	GDCLASS(PermissiveVisibilityInterface, RefCounted)

private:
	bool *losBlockerMap = nullptr;
	bool **visibilityMap = nullptr; // Note: a multidimensional array of pointers to multidimensional arrays of bools seems pretty inefficient. is this necessary?
	Vector2 currentOrigin;
	int width;
	int height;

	bool _is_in_bounds(int x, int y);

protected:
	static void _bind_methods();

public:
	void prepare_to_calculate_sightlines(PackedByteArray losBlockerData, Vector2 mapSize); // NOTE: why is mapSize Vector2 and not Vector2i?

	bool can_tile_see(Vector2 origin, Vector2 target); // NOTE: why are these arguments Vector2 and not Vector2i?

	void update_los_blocker_for_tile(int x, int y, bool tileBlocksVisibility);

	void clear_visibility_cache();

	bool *_calculate_sightlines_from_tile(int x, int y);
	TypedArray<bool> calculate_sightlines_from_tile(int x, int y);

	bool blocks_light(int x, int y);

	void set_visible(int x, int y);

	void clear_maps();

	PermissiveVisibilityInterface() = default;
	~PermissiveVisibilityInterface();
};
