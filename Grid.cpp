#include "Grid.h"

Grid::Grid() {
	this->type = 0;
	this->wall_ob = nullptr;
	this->ob = nullptr;
}

Grid::Grid(char type, Object* wall_ob) {
	this->type = type;
	this->wall_ob = wall_ob;
	this->ob = nullptr;
}

Grid::~Grid() {
	
}

char Grid::get_type() {
	return this->type;
}

Object* Grid::get_wall_ob() {
	return this->wall_ob;
}

Object* Grid::get_ob() {
	return this->ob;
}

void Grid::set_type(char type) {
	this->type = type;
}

void Grid::set_wall_ob(Object* wall_ob) {
	this->wall_ob = wall_ob;
}

void Grid::set_ob(Object* ob) {
	this->ob = ob;
}

bool Grid::check_grid_valid_location(Location loc) {
	return true;
}

char Grid::remove_ob_by_addr(Object* target) {
	
	Object* tp = this->ob;

	// Base case: No objects in grid
	if (tp == nullptr) return 0;

	// Base case: removing head
	else if (tp == target) {
		// Only one object
		if (tp->next == nullptr) {
			this->ob = nullptr;
			tp->next = nullptr;
			return 1;
		}

		// More than one object
		else {
			// Don't need to free
			this->ob = tp->next;
		}
	}

	while (tp->next != nullptr) {
		if (tp->next == target) {
			tp->next = target->next;
			target->next = nullptr;
			return 1;
		}
		tp = tp->next;
	}
	
	return 0;
}

// Add to head
void Grid::add_ob_by_addr(Object* target) {
	assert(target->next == nullptr);
	
	// Base case: Nothing in Grid
	if (this->ob == nullptr) {
		this->ob = target;
	}

	else {
		target->next = this->ob;
		this->ob = target;
	}

	return;
}
