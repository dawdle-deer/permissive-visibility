#pragma once

#include "godot_cpp/variant/callable.hpp"

#include "permissive_visibility_interface.h"

using namespace godot;

bool PermissiveVisibilityInterfaceGDExt::_is_in_bounds(int x, int y) {
	return x >= 0 && x < width && y >= 0 && y < height;
}

bool PermissiveVisibilityInterfaceGDExt::_is_map_valid() {
	return width > 0 && height > 0;
}

void PermissiveVisibilityInterfaceGDExt::prepare_to_calculate_sightlines(PackedByteArray losBlockerData, Vector2 mapSize) { // NOTE: why is mapSize Vector2 and not Vector2i?
	width = (int)mapSize.x;
	height = (int)mapSize.y;

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
		bool tileBlocksVisibility = losBlockerData[i] > 0;
		losBlockerMap[i] = tileBlocksVisibility;

		// set up the array to store sightlines from this tile
		// newVisMap[i] = nullptr; // new bool[width * height]
		newVisMap[i] = new bool[width * height];
	}

	visibilityMap = newVisMap;
}

bool PermissiveVisibilityInterfaceGDExt::can_tile_see(Vector2 origin, Vector2 target) { // NOTE: why are these arguments Vector2 and not Vector2i?
	ERR_FAIL_COND_V_MSG(!_is_map_valid(), false, "Visibility map is invalid!");

	bool *visibilityFromOrigin = visibilityMap[((int)origin.y * width) + (int)origin.x];
	if (visibilityFromOrigin == nullptr) {
		visibilityFromOrigin = _calculate_sightlines_from_tile((int)origin.x, (int)origin.y);
		visibilityMap[((int)origin.y * width) + (int)origin.x] = visibilityFromOrigin;
	}

	return visibilityFromOrigin[(int)target.x, (int)target.y];
}

void PermissiveVisibilityInterfaceGDExt::update_los_blocker_for_tile(int x, int y, bool tileBlocksVisibility) {
	ERR_FAIL_COND_MSG(!_is_map_valid(), "Visibility map is invalid!");
	// Only update the map if it's different than how it was
	if ((bool)losBlockerMap[(y * width) + x] != tileBlocksVisibility) {
		losBlockerMap[(y * width) + x] = tileBlocksVisibility;
		clear_visibility_cache();
	}
}

void PermissiveVisibilityInterfaceGDExt::clear_visibility_cache() {
	if (visibilityMap == nullptr) {
		return;
	}
	for (int i = 0; i < width * height; i++) {
		if (visibilityMap[i] != nullptr) {
			delete[] visibilityMap[i];
		}
		visibilityMap[i] = nullptr;
	}
}

bool *PermissiveVisibilityInterfaceGDExt::_calculate_sightlines_from_tile(int x, int y) {
	ERR_FAIL_COND_V_MSG(!_is_map_valid(), nullptr, "Visibility map is invalid!");

	// set up the array to store sightlines from this tile
	if (visibilityMap[(y * width) + x] != nullptr) {
		delete[] visibilityMap[(y * width) + x];
	}
	visibilityMap[(y * width) + x] = new bool[width * height];

	// TODO: find a better way to make a Callable than a string name?
	// it should *really* be callable_mp(this, set_visible), but something about that isn't appreciated by scons...
	PermissiveVisibilityCalculatorGDExt *visibilityCalculator = memnew(PermissiveVisibilityCalculatorGDExt);
	visibilityCalculator->BlocksLight = Callable::create(this, "blocks_light");
	visibilityCalculator->SetVisible = Callable::create(this, "set_visible");

	currentOrigin = Vector2(x, y);

	visibilityCalculator->compute(currentOrigin);

	return visibilityMap[((int)currentOrigin.y * width) + (int)currentOrigin.x];
}

TypedArray<bool> PermissiveVisibilityInterfaceGDExt::calculate_sightlines_from_tile(int x, int y) {
	return TypedArray<bool>(_calculate_sightlines_from_tile(x, y));
}

bool PermissiveVisibilityInterfaceGDExt::blocks_light(int x, int y) {
	ERR_FAIL_COND_V_MSG(!_is_map_valid(), false, "Visibility map is invalid!");
	return !_is_in_bounds(x, y) || losBlockerMap[(y * width) + x];
}

void PermissiveVisibilityInterfaceGDExt::set_visible(int x, int y) {
	ERR_FAIL_COND_MSG(!_is_map_valid(), "Tried to set tile visibility, but visibility map is invalid!");
	ERR_FAIL_COND_MSG(!_is_in_bounds(x, y), "Tried to set tile visibility, but coordinates were out of bounds!");
	bool *los_map = visibilityMap[((int)currentOrigin.y * width) + (int)currentOrigin.x];
	ERR_FAIL_COND_MSG(los_map == nullptr, "Tried to set tile visibility, but LOS map is invalid at the current origin tile!");
	los_map[(y * width) + x] = true;
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