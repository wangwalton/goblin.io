#include "Grid.h"

Grid::Grid() {
	this->type = 0;
	this->wall_ob = nullptr;
	this->pl = nullptr;
}

Grid::Grid(char type, Object* wall_ob) {
	this->type = type;
	this->wall_ob = wall_ob;
	this->pl = nullptr;
}

Grid::~Grid() {
	
}

char Grid::get_type() {
	return this->type;
}

Object* Grid::get_wall_ob() {
	return this->wall_ob;
}

MoveableObjectNode* Grid::get_monp() {
	return this->pl;
}

void Grid::set_type(char type) {
	this->type = type;
}

void Grid::set_wall_ob(Object* wall_ob) {
	this->wall_ob = wall_ob;
}

void Grid::set_monp(MoveableObjectNode* monp) {
	this->pl = monp;
}

bool Grid::check_grid_valid_location(Location loc) {
	return true;
}

char Grid::remove_ob_by_addr(Player* target) {
	
	MoveableObjectNode* tp = this->pl;

	// Base case: No objects in grid
	if (tp == nullptr) return 1;

	// Base case: removing head
	else if (tp->pl == target) {
		// Only one object
		if (tp->next == nullptr) {
			this->pl = nullptr;
			delete tp;
			return 0;
		}

		// More than one object
		else {
			// Don't need to free
			this->pl = tp->next;
			delete tp;
		}
	}

	while (tp->next != nullptr) {
		if (tp->next->pl == target) {
			MoveableObjectNode* temp = tp->next->next;
			delete tp->next;
			tp->next = temp;
			return 0;
		}
		tp = tp->next;
	}
	
	return 1;
}

// Add to head
void Grid::add_ob_by_addr(Player* target) {
	
	// Make a monp wrapper
	MoveableObjectNode *temp = new MoveableObjectNode{target, this->pl};
	this->pl = temp;
	return;
}
