#pragma once

#include "godot_cpp/variant/callable.hpp"

#include "permissive_visibility_interface.h"

using namespace godot;

inline bool PermissiveVisibilityInterfaceGDExt::_is_in_bounds(int x, int y) {
	return x >= 0 && x < width && y >= 0 && y < height;
}

inline bool PermissiveVisibilityInterfaceGDExt::_is_map_valid() {
	return width > 0 && height > 0;
}

inline int PermissiveVisibilityInterfaceGDExt::_to_map_index(int x, int y) {
	return (y * width) + x;
}

inline int PermissiveVisibilityInterfaceGDExt::_to_map_index(Vector2i pos) {
	return (pos.y * width) + pos.x;
}

void PermissiveVisibilityInterfaceGDExt::prepare_to_calculate_sightlines(PackedByteArray losBlockerData, Vector2i mapSize) {
	width = mapSize.x;
	height = mapSize.y;

	ERR_FAIL_COND_MSG(width <= 0 || height <= 0, "Tried to create map with invalid size!");

	ERR_FAIL_COND_MSG(
			losBlockerData.is_empty() || ((width * height) != losBlockerData.size()),
			"Tried to create map, but size of losBlockerData does not match mapSize or is empty!");

	clear_maps();

	// construct a 2D array out of the 1D array and map size we were passed
	losBlockerMap = new bool[width * height];
	bool **newVisMap = new bool *[width * height];

	// GD.Print("----- Setting up blockers and visibility arrays for map of size ", width,',',height);

	for (int i = 0; i < width * height; i++) {
		// store whether this tile blocks visibility
		losBlockerMap[i] = (bool)(losBlockerData[i] > 0);

		// set up the array to store sightlines from this tile
		newVisMap[i] = nullptr;
		// newVisMap[i] = new bool[width * height];
	}

	visibilityMap = newVisMap;
}

bool PermissiveVisibilityInterfaceGDExt::can_tile_see(Vector2i origin, Vector2i target) {
	ERR_FAIL_COND_V_MSG(!_is_map_valid(), false, "Visibility map is invalid!");

	bool *visibilityFromOrigin = visibilityMap[_to_map_index(origin)];

	if (visibilityFromOrigin == nullptr) {
		visibilityFromOrigin = _calculate_sightlines_from_tile(origin.x, origin.y);
		visibilityMap[_to_map_index(origin)] = visibilityFromOrigin;
	}

	return visibilityFromOrigin[_to_map_index(target)];
}

void PermissiveVisibilityInterfaceGDExt::update_los_blocker_for_tile(int x, int y, bool tileBlocksVisibility) {
	ERR_FAIL_COND_MSG(!_is_map_valid(), "Visibility map is invalid!");

	// Only update the map if tile's visibility is changed
	if (losBlockerMap[_to_map_index(x, y)] == tileBlocksVisibility) {
		return;
	}

	losBlockerMap[_to_map_index(x, y)] = tileBlocksVisibility;
	clear_visibility_cache();
}

void PermissiveVisibilityInterfaceGDExt::clear_visibility_cache() {
	if (visibilityMap == nullptr) {
		return;
	}
	for (int i = 0; i < width * height; i++) {
		if (visibilityMap[i] != nullptr) {
			delete[] visibilityMap[i];
			visibilityMap[i] = nullptr;
		}
	}
}

bool *PermissiveVisibilityInterfaceGDExt::_calculate_sightlines_from_tile(int x, int y) {
	ERR_FAIL_COND_V_MSG(!_is_map_valid(), nullptr, "Visibility map is invalid!");

	currentOrigin = Vector2i(x, y);

	// set up the array to store sightlines from this tile
	if (visibilityMap[_to_map_index(currentOrigin)] != nullptr) {
		delete[] visibilityMap[_to_map_index(currentOrigin)];
	}
	bool *los_map = new bool[width * height];
	memset(los_map, false, width * height);
	visibilityMap[_to_map_index(currentOrigin)] = los_map;

	PermissiveVisibilityCalculatorGDExt *visibilityCalculator = memnew(PermissiveVisibilityCalculatorGDExt);
	visibilityCalculator->BlocksLight = Callable::create(this, "blocks_light");
	visibilityCalculator->SetVisible = Callable::create(this, "set_visible");

	visibilityCalculator->compute(currentOrigin);

	memdelete(visibilityCalculator);

	return los_map;
}

TypedArray<bool> PermissiveVisibilityInterfaceGDExt::calculate_sightlines_from_tile(int x, int y) {
	TypedArray<bool> result = TypedArray<bool>();
	result.resize(width * height);
	bool *sightlines = _calculate_sightlines_from_tile(x, y);
	if (sightlines == nullptr) {
		return result;
	}
	for (int i = 0; i < width * height; i++) {
		result[i] = sightlines[i];
	}
	return result;
}

bool PermissiveVisibilityInterfaceGDExt::blocks_light(int x, int y) {
	ERR_FAIL_COND_V_MSG(!_is_map_valid(), false, "Visibility map is invalid!");
	return !_is_in_bounds(x, y) || losBlockerMap[_to_map_index(x, y)];
}

void PermissiveVisibilityInterfaceGDExt::set_visible(int x, int y) {
	ERR_FAIL_COND_MSG(!_is_map_valid(), "Tried to set tile visibility, but visibility map is invalid!");
	if (!_is_in_bounds(x, y)) {
		return;
	}
	bool *los_map = visibilityMap[_to_map_index(currentOrigin)];
	ERR_FAIL_COND_MSG(los_map == nullptr, "Tried to set tile visibility, but LOS map is invalid at the current origin tile!");
	los_map[_to_map_index(x, y)] = true;
}

void PermissiveVisibilityInterfaceGDExt::clear_maps() {
	if (losBlockerMap != nullptr) {
		delete[] losBlockerMap;
	}
	if (visibilityMap != nullptr) {
		for (int i = 0; i < width * height; i++) {
			if (visibilityMap[i] != nullptr) {
				delete[] visibilityMap[i];
			}
		}
		delete[] visibilityMap;
	}
}

PermissiveVisibilityInterfaceGDExt::~PermissiveVisibilityInterfaceGDExt() {
	clear_maps();
}

void PermissiveVisibilityInterfaceGDExt::_bind_methods() {
	godot::ClassDB::bind_method(D_METHOD("prepare_to_calculate_sightlines", "losBlockerData", "mapSize"), &PermissiveVisibilityInterfaceGDExt::prepare_to_calculate_sightlines);
	godot::ClassDB::bind_method(D_METHOD("can_tile_see", "origin", "target"), &PermissiveVisibilityInterfaceGDExt::can_tile_see);
	godot::ClassDB::bind_method(D_METHOD("update_los_blocker_for_tile", "x", "y", "tileBlocksVisibility"), &PermissiveVisibilityInterfaceGDExt::update_los_blocker_for_tile);
	godot::ClassDB::bind_method(D_METHOD("clear_visibility_cache"), &PermissiveVisibilityInterfaceGDExt::clear_visibility_cache);
	godot::ClassDB::bind_method(D_METHOD("calculate_sightlines_from_tile", "x", "y"), &PermissiveVisibilityInterfaceGDExt::calculate_sightlines_from_tile);
	godot::ClassDB::bind_method(D_METHOD("blocks_light", "x", "y"), &PermissiveVisibilityInterfaceGDExt::blocks_light);
	godot::ClassDB::bind_method(D_METHOD("set_visible", "x", "y"), &PermissiveVisibilityInterfaceGDExt::set_visible);
}