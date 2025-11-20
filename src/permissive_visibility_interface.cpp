#pragma once

#include "godot_cpp/variant/callable.hpp"

#include "permissive_visibility_interface.h"

using namespace godot;

void PermissiveVisibilityInterfaceGDExt::prepare_to_calculate_sightlines(PackedByteArray losBlockerData, Vector2i mapSize) {
	data.initialize_map(losBlockerData, mapSize);
}

bool PermissiveVisibilityInterfaceGDExt::can_tile_see(Vector2i origin, Vector2i target) {
	ERR_FAIL_COND_V_MSG(!data._is_map_valid(), false, "Visibility map is invalid!");

	bool *visibilityFromOrigin = data.visibilityMap[data._to_map_index(origin)];

	if (visibilityFromOrigin == nullptr) {
		visibilityFromOrigin = _calculate_sightlines_from_tile(origin.x, origin.y);
		data.visibilityMap[data._to_map_index(origin)] = visibilityFromOrigin;
	}

	return visibilityFromOrigin[data._to_map_index(target)];
}

void PermissiveVisibilityInterfaceGDExt::update_los_blocker_for_tile(int x, int y, bool tileBlocksVisibility) {
	ERR_FAIL_COND_MSG(!data._is_map_valid(), "Visibility map is invalid!");

	// Only update the map if tile's visibility is changed
	if (data.losBlockerMap[data._to_map_index(x, y)] == tileBlocksVisibility) {
		return;
	}

	data.losBlockerMap[data._to_map_index(x, y)] = tileBlocksVisibility;
	clear_visibility_cache();
}

void PermissiveVisibilityInterfaceGDExt::clear_visibility_cache() {
	data.clear_visibility_cache();
}

bool *PermissiveVisibilityInterfaceGDExt::_calculate_sightlines_from_tile(int x, int y) {
	ERR_FAIL_COND_V_MSG(!data._is_map_valid(), nullptr, "Visibility map is invalid!");

	data.currentOrigin = Vector2i(x, y);

	// set up the array to store sightlines from this tile
	if (data.visibilityMap[data._to_map_index(data.currentOrigin)] != nullptr) {
		delete[] data.visibilityMap[data._to_map_index(data.currentOrigin)];
	}
	bool *los_map = new bool[data.width * data.height];
	memset(los_map, false, data.width * data.height);
	data.visibilityMap[data._to_map_index(data.currentOrigin)] = los_map;

	PermissiveVisibilityCalculatorGDExt *visibilityCalculator = memnew(PermissiveVisibilityCalculatorGDExt);
	visibilityCalculator->data_reference = &data;

	visibilityCalculator->compute(data.currentOrigin);

	memdelete(visibilityCalculator);

	return los_map;
}

TypedArray<bool> PermissiveVisibilityInterfaceGDExt::calculate_sightlines_from_tile(int x, int y) {
	TypedArray<bool> result = TypedArray<bool>();
	result.resize(data.width * data.height);
	bool *sightlines = _calculate_sightlines_from_tile(x, y);
	if (sightlines == nullptr) {
		return result;
	}
	for (int i = 0; i < data.width * data.height; i++) {
		result[i] = sightlines[i];
	}
	return result;
}

bool PermissiveVisibilityInterfaceGDExt::blocks_light(int x, int y) {
	return data.blocks_light(x, y);
}

void PermissiveVisibilityInterfaceGDExt::set_visible(int x, int y) {
	data.set_visible(x, y);
}

void PermissiveVisibilityInterfaceGDExt::clear_maps() {
	data.clear_maps();
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