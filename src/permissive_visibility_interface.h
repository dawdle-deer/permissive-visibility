#pragma once

#include "godot_cpp/classes/ref_counted.hpp"

#include "permissive_visibility_calculator.h"
#include "permissive_visibility_data.h"

using namespace godot;

class PermissiveVisibilityInterfaceGDExt : public RefCounted {
	GDCLASS(PermissiveVisibilityInterfaceGDExt, RefCounted)

private:
	PermissiveVisibilityDataGDExt data;

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
