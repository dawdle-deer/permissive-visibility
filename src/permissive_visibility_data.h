#pragma once

#include "godot_cpp/classes/ref_counted.hpp"

using namespace godot;

class PermissiveVisibilityDataGDExt {
public:
	bool *losBlockerMap = nullptr;
	bool **visibilityMap = nullptr; // Note: a multidimensional array of pointers to multidimensional arrays of bools seems pretty inefficient. is this necessary?
	Vector2i currentOrigin = Vector2i(0, 0);
	int width = 0;
	int height = 0;

	inline bool _is_in_bounds(int x, int y);
	inline bool _is_map_valid();
	inline int _to_map_index(int x, int y);
	inline int _to_map_index(Vector2i pos);

	void initialize_map(PackedByteArray losBlockerData, Vector2i mapSize);

	bool blocks_light(int x, int y);

	void set_visible(int x, int y);

	void clear_maps();

	void clear_visibility_cache();
};