#include "Map.h"

void Map::print_map() {

	// Printing a top boarder
	for (int tb = 0; tb < MAP_WIDTH+2; tb++) {
		std::cout << '_';
	}
	std::cout << std::endl;
	// Rows
	for (int i = 0; i < MAP_HEIGHT; i++) {

		// Printing left boader
		std::cout << '|';
		// Columns
		for (int j = 0; j < MAP_WIDTH; j++) {
			Location actual (i, j);
			Location offset (i % REP_FACTOR, j % REP_FACTOR);
			bool printed = 0;
			// PRINTING WALL
			if (this->has_walls) {
				// Whole Block is wall
				if (map[i / REP_FACTOR][j / REP_FACTOR].get_type() == 1) {
				
					// Print wall rep_factor type and increment j
					for (int k = 0; j < REP_FACTOR; j++) {
						std::cout << WALL;	
						printed = 1;
					}
					j += REP_FACTOR;
				}

				// Check if PT is a polygon in wall
				if (!map[i / REP_FACTOR][j / REP_FACTOR].check_grid_valid_location(offset)) {
					std::cout << WALL;
					printed = 1;
				}

			}
			
			// Printing Player /
			Object* ob = map[i / REP_FACTOR][j / REP_FACTOR].get_ob();
			while (ob != nullptr) {
				if (ob->get_loc() == actual) {
					if (ob->get_type() == PLAYER_TYPE) {
						std::cout << PLAYER;
						printed = 1;
						break;
					}
					else if (ob->get_type() == MONSTER_TYPE) {
						std::cout << MONSTER;
						printed = 1;
						break;
					}
				}
				ob = ob->next;
			}

			if (printed == 0) {
				std::cout << SPACE;
			}
		}
		std::cout << '|' <<std::endl;
	}

	// Printing bottom boarder
	for (int bb = 0; bb < MAP_WIDTH+2; bb++) {
		std::cout << '_';
	}
	std::cout << std::endl;
}

char Map::move_object(Object* ob, Location loc) {
	char ret_val = 1;
	
	int grid_i_f = std::get<0>(loc) / REP_FACTOR;
	int grid_j_f = std::get<1>(loc) / REP_FACTOR;
	
	Location loc_i = ob->get_loc();
	int grid_i_i = std::get<0>(loc_i) / REP_FACTOR;
	int grid_j_i = std::get<1>(loc_i) / REP_FACTOR;

	// Location mismatch;
	if (grid_i_i != grid_i_f ||grid_j_i != grid_j_f) {
		if (this->map[grid_i_i][grid_j_i].remove_ob_by_addr(ob)) {
			std::cout << "Object doesn't exist in grid: (" << grid_i_i 
					  << ", " << grid_j_i << ")." << std::endl;
			ret_val = 0;

		}
		else {
			this->map[grid_i_f][grid_j_f].add_ob_by_addr(ob);
		}
	}
	return ret_val;
}

char Map::add_object(Object* ob, Location loc) {

	int grid_i_f = std::get<0>(loc) / REP_FACTOR;
	int grid_j_f = std::get<1>(loc) / REP_FACTOR;
	this->map[grid_i_f][grid_j_f].add_ob_by_addr(ob);
	return 0;

}
bool Map::check_valid_location(Location loc) {
	int grid_i = std::get<0>(loc) / REP_FACTOR;
	int grid_j = std::get<1>(loc) / REP_FACTOR;

	Location offset_loc (std::get<0>(loc) % REP_FACTOR, 
						 std::get<1>(loc) % REP_FACTOR);

	if (this->map[grid_i][grid_j].check_grid_valid_location(offset_loc))
		return true;

	return false;
}
