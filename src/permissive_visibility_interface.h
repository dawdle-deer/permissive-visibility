#pragma once

#include "godot_cpp/classes/ref_counted.hpp"

#include "permissive_visibility_calculator.h"

using namespace godot;

class PermissiveVisibilityInterfaceGDExt : public RefCounted {
	GDCLASS(PermissiveVisibilityInterfaceGDExt, RefCounted)

private:
	bool *losBlockerMap = nullptr;
	bool **visibilityMap = nullptr; // Note: a multidimensional array of pointers to multidimensional arrays of bools seems pretty inefficient. is this necessary?
	Vector2i currentOrigin = Vector2i(0, 0);
	int width = 0;
	int height = 0;

	bool _is_in_bounds(int x, int y);
	bool _is_map_valid();

protected:
	static void _bind_methods();

public:
	void prepare_to_calculate_sightlines(PackedByteArray losBlockerData, Vector2i mapSize); // NOTE: why is mapSize Vector2 and not Vector2i?

	bool can_tile_see(Vector2i origin, Vector2i target); // NOTE: why are these arguments Vector2 and not Vector2i?

	void update_los_blocker_for_tile(int x, int y, bool tileBlocksVisibility);

	void clear_visibility_cache();

	bool *_calculate_sightlines_from_tile(int x, int y);
	TypedArray<bool> calculate_sightlines_from_tile(int x, int y);

	bool blocks_light(int x, int y);

	void set_visible(int x, int y);

	void clear_maps();

	PermissiveVisibilityInterfaceGDExt() = default;
	~PermissiveVisibilityInterfaceGDExt();
};
