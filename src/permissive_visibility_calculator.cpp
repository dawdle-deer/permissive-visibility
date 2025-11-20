
// Based on code by Adam Mil at http://www.adammil.net/blog/v125_roguelike_vision_algorithms.html#permissivecode

#pragma once

#include "godot_cpp/classes/ref_counted.hpp"
#include "godot_cpp/templates/list.hpp"
#include <climits>

#include "permissive_visibility_calculator.h"

using namespace godot;

inline bool PermissiveVisibilityCalculatorGDExt::Line::is_below(Vector2i point) {
	return relative_slope(point) > 0;
}

inline bool PermissiveVisibilityCalculatorGDExt::Line::is_below_or_contains(Vector2i point) {
	return relative_slope(point) >= 0;
}

inline bool PermissiveVisibilityCalculatorGDExt::Line::is_above(Vector2i point) {
	return relative_slope(point) < 0;
}

inline bool PermissiveVisibilityCalculatorGDExt::Line::is_above_or_contains(Vector2i point) {
	return relative_slope(point) <= 0;
}

inline bool PermissiveVisibilityCalculatorGDExt::Line::does_contain(Vector2i point) {
	return relative_slope(point) == 0;
}

// negative if the line is above the point.
// positive if the line is below the point.
// 0 if the line is on the point.
inline int PermissiveVisibilityCalculatorGDExt::Line::relative_slope(Vector2i point) {
	return (far.y - near.y) * (far.x - point.x) - (far.y - point.y) * (far.x - near.x);
}

void PermissiveVisibilityCalculatorGDExt::compute(Vector2i origin) // , int rangeLimit)
{
	// print_line("Computing Visibility");
	source = origin;
	//	this.rangeLimit = rangeLimit;
	for (unsigned short q = 0; q < 4; q++) {
		// 1 1    -1 1    -1 -1     1 -1
		// print_line("Computing Quadrant ", q);
		quadrant.x = ((q == 0 || q == 3) ? 1 : -1);
		quadrant.y = (q < 2 ? 1 : -1);
		compute_quadrant();
	}
}

void PermissiveVisibilityCalculatorGDExt::compute_quadrant() {
	const int Infinity = SHRT_MAX;
	List<Field> activeFields = List<Field>();
	activeFields.push_back(
			Field{
					Line{ Vector2i{ 1, 0 }, Vector2i{ 0, Infinity } },
					Line{ Vector2i{ 0, 1 }, Vector2i{ Infinity, 0 } } });

	Vector2i dest = Vector2i(0, 0);
	act_is_blocked(dest);
	for (short i = 1; i < Infinity && activeFields.size() > 0; i++) {
		List<Field>::Element *current = activeFields.front();
		for (short j = 0; j <= i && current != nullptr; j++) {
			dest.x = i - j;
			dest.y = j;
			current = visit_square(dest, current, &activeFields);
		}
	}
}

bool PermissiveVisibilityCalculatorGDExt::act_is_blocked(Vector2i pos) {
	int x = pos.x * quadrant.x + source.x, y = pos.y * quadrant.y + source.y;
	SetVisible.call(x, y);
	return BlocksLight.call(x, y);
}

List<PermissiveVisibilityCalculatorGDExt::Field>::Element *PermissiveVisibilityCalculatorGDExt::visit_square(Vector2i dest, List<Field>::Element *currentField, List<Field> *activeFields) {
	// print_line("\tvisit_square (", dest.x, ", ", dest.y, ") ", (int)&currentField->get());
	Vector2i topLeft = Vector2i(dest.x, (short)(dest.y + 1));
	Vector2i bottomRight = Vector2i((short)(dest.x + 1), (short)dest.y);
	while (currentField != nullptr && currentField->get().steep.is_below_or_contains(bottomRight)) {
		currentField = currentField->next();
	}

	if (currentField == nullptr || currentField->get().shallow.is_above_or_contains(topLeft) || !act_is_blocked(dest)) {
		return currentField;
	}

	Field value = currentField->get();

	if (value.shallow.is_above(bottomRight) && value.steep.is_below(topLeft)) {
		List<Field>::Element *next = currentField->next();
		activeFields->erase(currentField);
		return next;
	} else if (value.shallow.is_above(bottomRight)) {
		add_shallow_bump(topLeft, currentField);
		return check_field(currentField, activeFields);
	} else if (value.steep.is_below(topLeft)) {
		add_steep_bump(bottomRight, currentField);
		return check_field(currentField, activeFields);
	} else {
		List<Field>::Element *steeper = currentField;
		List<Field>::Element *shallower = activeFields->insert_before(currentField, value);
		add_steep_bump(bottomRight, shallower);
		check_field(shallower, activeFields);
		add_shallow_bump(topLeft, steeper);
		return check_field(steeper, activeFields);
	}
}

void PermissiveVisibilityCalculatorGDExt::add_shallow_bump(Vector2i point, List<Field>::Element *currentField) {
	Field value = currentField->get();
	value.shallow.far = point;
	List<Vector2i>::Element *currentBump = value.steepBumps.front();
	value.shallowBumps.push_front(point);
	// Look through the list of steep bumps and see if any of them are above the line.
	while (currentBump != nullptr) {
		if (value.shallow.is_above(currentBump->get())) {
			value.shallow.near = currentBump->get();
		}
		currentBump = currentBump->next();
	}
	currentField->set(value);
}

void PermissiveVisibilityCalculatorGDExt::add_steep_bump(Vector2i point, List<Field>::Element *currentField) {
	Field value = currentField->get();
	value.steep.far = point;
	List<Vector2i>::Element *currentBump = value.shallowBumps.front();
	value.steepBumps.push_front(point);
	// Look through the list of shallow bumps and see if any of them are below the line.
	while (currentBump != nullptr) {
		if (value.steep.is_below(currentBump->get())) {
			value.steep.near = currentBump->get();
		}
		currentBump = currentBump->next();
	}
	currentField->set(value);
}

List<PermissiveVisibilityCalculatorGDExt::Field>::Element *PermissiveVisibilityCalculatorGDExt::check_field(List<Field>::Element *currentField, List<Field> *activeFields) {
	Field value = currentField->get();
	List<Field>::Element *result = currentField;
	if (value.shallow.does_contain(value.steep.near) &&
			value.shallow.does_contain(value.steep.far) &&
			(value.shallow.does_contain(Vector2i(0, 1)) || value.shallow.does_contain(Vector2i(1, 0)))) {
		result = result->next();
		activeFields->erase(currentField);
	}
	return result;
}

void PermissiveVisibilityCalculatorGDExt::_bind_methods() {
	godot::ClassDB::bind_method(D_METHOD("compute", "origin"), &PermissiveVisibilityCalculatorGDExt::compute);
	godot::ClassDB::bind_method(D_METHOD("compute_quadrant"), &PermissiveVisibilityCalculatorGDExt::compute_quadrant);
	// godot::ClassDB::bind_method(D_METHOD("act_is_blocked", "position"), &PermissiveVisibilityCalculator::act_is_blocked);
	// godot::ClassDB::bind_method(D_METHOD("visit_square", "destination", "currentField", "activeFields"), &PermissiveVisibilityCalculator::visit_square);
	// godot::ClassDB::bind_method(D_METHOD("add_shallow_bump", "position", "currentField"), &PermissiveVisibilityCalculator::add_shallow_bump);
	// godot::ClassDB::bind_method(D_METHOD("add_steep_bump", "position", "currentField"), &PermissiveVisibilityCalculator::add_steep_bump);
	// godot::ClassDB::bind_method(D_METHOD("check_field", "currentField", "activeFields"), &PermissiveVisibilityCalculator::check_field);
}