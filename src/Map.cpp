#include "Map.h"

bool is_tile_within_radius(Location t, Location c, int rad) {
	int d_x = std::get<0>(t) - std::get<0>(c);
	int d_y = std::get<1>(t) - std::get<1>(c);

	int dist_2 = d_x * d_x + d_y * d_y;
	return dist_2 < rad * rad;
}

void Map::print_map() {

	// Printing a top boarder
	for (int tb = 0; tb < MAP_WIDTH+2; tb++) {
		std::cout << HORI_BORDER;
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
			
			// Printing objects
			Player* ob = map[i / REP_FACTOR][j / REP_FACTOR].get_ob();
			while (ob != nullptr) {

				// Checks if distance matches current tile
				if (is_tile_within_radius(ob->get_loc(), actual, ob->get_rad())) {
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
		std::cout << HORI_BORDER;
	}
	std::cout << std::endl;
	return;
}

char Map::move_object(Player* ob, Location loc) {
	
	char ret_val = 1;

	int out_val = this->remove_object(ob);
	if (out_val != 0) {
		std::cout << "Object doesn't exist in grid. Error code: " 
				  << out_val << std::endl;
		ret_val = 0;

	}

	// Removal success, time to add object to map again
	else {
		this->add_object(ob, loc);
	}

	
	return ret_val;
}

char Map::remove_object(Player* ob) {
	
	char ret_val = 1;

	char* bitmap = ob->get_map_rep();

	Location loc = ob->get_loc();
	int grid_i = std::get<0>(loc) / REP_FACTOR;
	int grid_j = std::get<1>(loc) / REP_FACTOR;

	for (int i = -1; i < 2; i++) {
		for (int j = -1; j < 2; j++) {
			if (bitmap[(i+1)*3+(j+1)] == 1) {
				// Returns 1 if unsuccessful
				if (this->map[grid_i+i][grid_j+j].remove_ob_by_addr(ob)) {
					ret_val = 2;
					return ret_val;
				}
				ret_val = 0;
				bitmap[(i+1)*3+(j+1)] = 0;
			}
		}
	}

	ob->set_map_rep(bitmap);
	return ret_val;
}

// Need to implement: Check if collides with wall
char Map::add_object(Player* ob, Location loc) {

	int ret_val = 0;

	char* bitmap = ob->get_map_rep();
	int grid_i_f = std::get<0>(loc) / REP_FACTOR;
	int grid_j_f = std::get<1>(loc) / REP_FACTOR;
	this->map[grid_i_f][grid_j_f].add_ob_by_addr(ob);
	bitmap[4] = 1;

	// Need to add not just the center piece
	int off_i = std::get<0>(loc) % REP_FACTOR;
	int off_j = std::get<1>(loc) % REP_FACTOR;
	
	// This is the gap between the representation rectangle
	// and the object center
	int left_relative_gap = off_i;
	int right_relative_gap = REP_FACTOR - off_i;
	int top_relative_gap = off_j;
	int bot_relative_gap = REP_FACTOR - off_j;
	
	// corner relative gaps, l(left), t(top), r(right), b(bottom);
	float ltrg = sqrt(left_relative_gap * left_relative_gap +
					  top_relative_gap * top_relative_gap);
	float trrg = sqrt(top_relative_gap * top_relative_gap +
					  right_relative_gap * right_relative_gap);
	float rbrg = sqrt(right_relative_gap * right_relative_gap +
					  bot_relative_gap * bot_relative_gap);
	float blrg = sqrt(bot_relative_gap * bot_relative_gap +
					  left_relative_gap * left_relative_gap);
	
	int ob_rad = ob->get_rad();
	int remaining_left_rad =  ob_rad - left_relative_gap;
	int remaining_right_rad =  ob_rad - right_relative_gap;
	int remaining_top_rad =  ob_rad - top_relative_gap;
	int remaining_bot_rad =  ob_rad - bot_relative_gap;
	
	// Naming convention: (left) (top) relative gap;
	float ltrr = ob_rad - ltrg;
	float trrr = ob_rad - trrg;
	float rbrr = ob_rad - rbrg;
	float blrr = ob_rad - blrg;

	assert(ltrr - REP_FACTOR < 0);
	assert(trrr - REP_FACTOR < 0);
	assert(rbrr - REP_FACTOR < 0);
	assert(blrr - REP_FACTOR < 0);
	assert(remaining_left_rad - REP_FACTOR < 0);
	assert(remaining_right_rad - REP_FACTOR < 0);
	assert(remaining_top_rad - REP_FACTOR < 0);
	assert(remaining_bot_rad - REP_FACTOR < 0);

	// Left top tile
	if (ltrr > 0) {
		this->map[grid_i_f-1][grid_j_f-1].add_ob_by_addr(ob);
		bitmap[0] = 1;
	}

	// Top right tile
	if (trrr > 0) {
		this->map[grid_i_f-1][grid_j_f+1].add_ob_by_addr(ob);
		bitmap[2] = 1;
	}

	// Right bot tile
	if (rbrr > 0) {
		this->map[grid_i_f+1][grid_j_f+1].add_ob_by_addr(ob);
		bitmap[8] = 1;
	}

	// Bot left tile
	if (blrr > 0) {
		this->map[grid_i_f+1][grid_j_f-1].add_ob_by_addr(ob);
		bitmap[6] = 1;
	}

	// Left tile
	if (remaining_left_rad > 0) {
		this->map[grid_i_f][grid_j_f-1].add_ob_by_addr(ob);
		bitmap[3] = 1;
	}

	// Top tile
	if (remaining_top_rad > 0) {
		this->map[grid_i_f-1][grid_j_f].add_ob_by_addr(ob);
		bitmap[1] = 1;
	}
	
	// Right tile
	if (remaining_right_rad > 0) {
		this->map[grid_i_f][grid_j_f+1].add_ob_by_addr(ob);
		bitmap[5] = 1;
	}
	
	// Bot tile
	if (remaining_left_rad > 0) {
		this->map[grid_i_f+1][grid_j_f].add_ob_by_addr(ob);
		bitmap[7] = 1;
	}
	
	ob->set_map_rep(bitmap);

	return ret_val;

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
