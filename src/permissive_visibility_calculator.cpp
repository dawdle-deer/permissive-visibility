
// Based on code by Adam Mil at http://www.adammil.net/blog/v125_roguelike_vision_algorithms.html#permissivecode

#pragma once

#include "godot_cpp/classes/ref_counted.hpp"
#include "godot_cpp/templates/list.hpp"
#include <climits>

#include "permissive_visibility_calculator.h"

using namespace godot;

void PermissiveVisibilityCalculator::compute(Vector2 origin) // , int rangeLimit)
{
	source = Offset((int)origin.x, (int)origin.y);
	//	this.rangeLimit = rangeLimit;
	for (int q = 0; q < 4; q++) {
		// 1 1    -1 1    -1 -1     1 -1
		quadrant.x = (short)(q == 0 || q == 3 ? 1 : -1);
		quadrant.y = (short)(q < 2 ? 1 : -1);
		compute_quadrant();
	}
}

PermissiveVisibilityCalculator::Bump::Bump(Bump *parent, Offset location) {
	this->parent = parent;
	this->location = location;
}

PermissiveVisibilityCalculator::Line::Line(Offset near, Offset far) {
	this->near = near;
	this->far = far;
}

bool PermissiveVisibilityCalculator::Line::is_below(Offset point) {
	return relative_slope(point) > 0;
}

bool PermissiveVisibilityCalculator::Line::is_below_or_contains(Offset point) {
	return relative_slope(point) >= 0;
}

bool PermissiveVisibilityCalculator::Line::is_above(Offset point) {
	return relative_slope(point) < 0;
}

bool PermissiveVisibilityCalculator::Line::is_above_or_contains(Offset point) {
	return relative_slope(point) <= 0;
}

bool PermissiveVisibilityCalculator::Line::does_contain(Offset point) {
	return relative_slope(point) == 0;
}

// negative if the line is above the point.
// positive if the line is below the point.
// 0 if the line is on the point.
int PermissiveVisibilityCalculator::Line::relative_slope(Offset point) {
	return (far.y - near.y) * (far.x - point.x) - (far.y - point.y) * (far.x - near.x);
}

PermissiveVisibilityCalculator::Offset::Offset(int x, int y) {
	this->x = (short)x;
	this->y = (short)y;
}

void PermissiveVisibilityCalculator::compute_quadrant() {
	const int Infinity = SHRT_MAX;
	List<Field> activeFields;
	activeFields.push_back(
			Field{
					new Bump(),
					new Bump(),
					Line(Offset(1, 0), Offset(0, Infinity)),
					Line(Offset(0, 1), Offset(Infinity, 0)) });

	Offset dest = Offset(0, 0);
	act_is_blocked(dest);
	for (int i = 1; i < Infinity && activeFields.size() != 0; i++) {
		List<Field>::Element *current = activeFields.front();
		for (int j = 0; j <= i; j++) {
			dest.x = (short)(i - j);
			dest.y = (short)j;
			current = visit_square(dest, current, &activeFields);
		}
	}
}

bool PermissiveVisibilityCalculator::act_is_blocked(Offset pos) {
	int x = pos.x * quadrant.x + source.x, y = pos.y * quadrant.y + source.y;
	SetVisible.call(x, y);
	return BlocksLight.call(x, y);
}

List<PermissiveVisibilityCalculator::Field>::Element *PermissiveVisibilityCalculator::visit_square(Offset dest, List<Field>::Element *currentField, List<Field> *activeFields) {
	Offset topLeft = Offset(dest.x, dest.y + 1), bottomRight = Offset(dest.x + 1, dest.y);
	while (currentField != nullptr && currentField->get().steep.is_below_or_contains(bottomRight))
		currentField = currentField->next();

	if (currentField == nullptr || currentField->get().shallow.is_above_or_contains(topLeft) || !act_is_blocked(dest))
		return currentField;

	if (currentField->get().shallow.is_above(bottomRight) && currentField->get().steep.is_below(topLeft)) {
		List<Field>::Element *next = currentField->next();
		activeFields->erase(currentField);
		return next;
	} else if (currentField->get().shallow.is_above(bottomRight)) {
		add_shallow_bump(topLeft, currentField);
		return check_field(currentField, activeFields);
	} else if (currentField->get().steep.is_below(topLeft)) {
		add_steep_bump(bottomRight, currentField);
		return check_field(currentField, activeFields);
	} else {
		List<Field>::Element *steeper = currentField, *shallower = activeFields->insert_before(currentField, currentField->get());
		add_steep_bump(bottomRight, shallower);
		check_field(shallower, activeFields);
		add_shallow_bump(topLeft, steeper);
		return check_field(steeper, activeFields);
	}
}

void PermissiveVisibilityCalculator::add_shallow_bump(Offset point, List<Field>::Element *currentField) {
	Field value = currentField->get();
	value.shallow.far = point;
	value.shallowBump = new Bump(value.shallowBump, point);

	Bump *currentBump = value.steepBump;
	while (currentBump != nullptr) {
		if (value.shallow.is_above(currentBump->location))
			value.shallow.near = currentBump->location;
		currentBump = currentBump->parent;
	}
	currentField->get() = value;
}

void PermissiveVisibilityCalculator::add_steep_bump(Offset point, List<Field>::Element *currentField) {
	Field value = currentField->get();
	value.steep.far = point;
	value.steepBump = new Bump(value.steepBump, point);
	// Now look through the list of shallow bumps and see if any of them are below the line.
	for (Bump *currentBump = value.shallowBump; currentBump != nullptr; currentBump = currentBump->parent) {
		if (value.steep.is_below(currentBump->location))
			value.steep.near = currentBump->location;
	}
	currentField->get() = value;
}

List<PermissiveVisibilityCalculator::Field>::Element *PermissiveVisibilityCalculator::check_field(List<Field>::Element *currentField, List<Field> *activeFields) {
	List<Field>::Element *result = currentField;
	if (currentField->get().shallow.does_contain(currentField->get().steep.near) &&
			currentField->get().shallow.does_contain(currentField->get().steep.far) &&
			(currentField->get().shallow.does_contain(Offset(0, 1)) || currentField->get().shallow.does_contain(Offset(1, 0)))) {
		result = result->next();
		activeFields->erase(currentField);
	}
	return result;
}

void PermissiveVisibilityCalculator::_bind_methods() {
	godot::ClassDB::bind_method(D_METHOD("compute", "origin"), &PermissiveVisibilityCalculator::compute);
	godot::ClassDB::bind_method(D_METHOD("compute_quadrant"), &PermissiveVisibilityCalculator::compute_quadrant);
	// godot::ClassDB::bind_method(D_METHOD("act_is_blocked", "position"), &PermissiveVisibilityCalculator::act_is_blocked);
	// godot::ClassDB::bind_method(D_METHOD("visit_square", "destination", "currentField", "activeFields"), &PermissiveVisibilityCalculator::visit_square);
	// godot::ClassDB::bind_method(D_METHOD("add_shallow_bump", "position", "currentField"), &PermissiveVisibilityCalculator::add_shallow_bump);
	// godot::ClassDB::bind_method(D_METHOD("add_steep_bump", "position", "currentField"), &PermissiveVisibilityCalculator::add_steep_bump);
	// godot::ClassDB::bind_method(D_METHOD("check_field", "currentField", "activeFields"), &PermissiveVisibilityCalculator::check_field);
}