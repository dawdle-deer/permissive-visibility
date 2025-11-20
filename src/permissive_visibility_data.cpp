#pragma once

#include "permissive_visibility_data.h"
#include "godot_cpp/classes/ref_counted.hpp"

using namespace godot;

inline bool PermissiveVisibilityDataGDExt::_is_in_bounds(int x, int y) {
	return x >= 0 && x < width && y >= 0 && y < height;
}

inline bool PermissiveVisibilityDataGDExt::_is_map_valid() {
	return width > 0 && height > 0;
}

inline int PermissiveVisibilityDataGDExt::_to_map_index(int x, int y) {
	return (y * width) + x;
}

inline int PermissiveVisibilityDataGDExt::_to_map_index(Vector2i pos) {
	return (pos.y * width) + pos.x;
}

void PermissiveVisibilityDataGDExt::initialize_map(PackedByteArray losBlockerData, Vector2i mapSize) {
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
		// newVisMap[i] = new bool[data.width * data.height];
	}

	visibilityMap = newVisMap;
}

bool PermissiveVisibilityDataGDExt::blocks_light(int x, int y) {
	ERR_FAIL_COND_V_MSG(!_is_map_valid(), false, "Visibility map is invalid!");
	return !_is_in_bounds(x, y) || losBlockerMap[_to_map_index(x, y)];
}

void PermissiveVisibilityDataGDExt::set_visible(int x, int y) {
	ERR_FAIL_COND_MSG(!_is_map_valid(), "Tried to set tile visibility, but visibility map is invalid!");
	if (!_is_in_bounds(x, y)) {
		return;
	}
	bool *los_map = visibilityMap[_to_map_index(currentOrigin)];
	ERR_FAIL_COND_MSG(los_map == nullptr, "Tried to set tile visibility, but LOS map is invalid at the current origin tile!");
	los_map[_to_map_index(x, y)] = true;
}

void PermissiveVisibilityDataGDExt::clear_maps() {
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

void PermissiveVisibilityDataGDExt::clear_visibility_cache() {
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