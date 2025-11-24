#include "permissive_visibility_data.h"
#include "godot_cpp/classes/ref_counted.hpp"

using namespace godot;

extern "C" {
bool PermissiveVisibilityDataGDExt::is_in_bounds(int x, int y) {
	return x >= 0 && x < width && y >= 0 && y < height;
}

bool PermissiveVisibilityDataGDExt::is_map_valid() {
	return width > 0 && height > 0;
}

int PermissiveVisibilityDataGDExt::to_map_index(int x, int y) {
	return (y * width) + x;
}

int PermissiveVisibilityDataGDExt::to_map_index_v(Vector2i pos) {
	return (pos.y * width) + pos.x;
}
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
	ERR_FAIL_COND_V_MSG(!is_map_valid(), false, "Visibility map is invalid!");
	return !is_in_bounds(x, y) || losBlockerMap[to_map_index(x, y)];
}

void PermissiveVisibilityDataGDExt::set_visible(int x, int y) {
	ERR_FAIL_COND_MSG(!is_map_valid(), "Tried to set tile visibility, but visibility map is invalid!");
	if (!is_in_bounds(x, y)) {
		return;
	}
	bool *los_map = visibilityMap[to_map_index_v(currentOrigin)];
	ERR_FAIL_COND_MSG(los_map == nullptr, "Tried to set tile visibility, but LOS map is invalid at the current origin tile!");
	los_map[to_map_index(x, y)] = true;
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

void PermissiveVisibilityDataGDExt::_bind_methods() {
	godot::ClassDB::bind_method(D_METHOD("is_in_bounds", "x", "y"), &PermissiveVisibilityDataGDExt::is_in_bounds);
	godot::ClassDB::bind_method(D_METHOD("is_map_valid"), &PermissiveVisibilityDataGDExt::is_map_valid);
	godot::ClassDB::bind_method(D_METHOD("to_map_index", "x", "y"), &PermissiveVisibilityDataGDExt::to_map_index);
	godot::ClassDB::bind_method(D_METHOD("to_map_index", "v"), &PermissiveVisibilityDataGDExt::to_map_index);
	godot::ClassDB::bind_method(D_METHOD("initialize_map", "losBlockerData", "mapSize"), &PermissiveVisibilityDataGDExt::initialize_map);
	godot::ClassDB::bind_method(D_METHOD("blocks_light", "x", "y"), &PermissiveVisibilityDataGDExt::blocks_light);
	godot::ClassDB::bind_method(D_METHOD("set_visible", "x", "y"), &PermissiveVisibilityDataGDExt::set_visible);
	godot::ClassDB::bind_method(D_METHOD("clear_maps"), &PermissiveVisibilityDataGDExt::clear_maps);
	godot::ClassDB::bind_method(D_METHOD("clear_visibility_cache"), &PermissiveVisibilityDataGDExt::clear_visibility_cache);
}