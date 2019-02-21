#ifndef _map_h
#define _map_h

#include "Grid.h"
#include "Player.h"
#include <iostream>
#include <cmath>
// Map constants
#define MAP_WIDTH 50
#define MAP_HEIGHT 25

// CONSTRAINT: Object radius cannot exceed REP_FACTOR
#define REP_FACTOR 5
#define WALL "@ "
#define SPACE "  "
#define PLAYER "P "
#define MONSTER "M "
#define HORI_BORDER "__"
#define MONSTER_TYPE 1
#define PLAYER_TYPE 0
#define SECONDS_PER_UPDATE 0.03
class Player;
class Map {
	public:
		// Right now only checks objects
		Grid map[MAP_HEIGHT / REP_FACTOR][MAP_WIDTH / REP_FACTOR];
		// Short cut
		char has_walls = 0;
		void print_map();
		
		// Simply moves an object within map, doesn't update internal
		// Object location or perform checking
		char move_object(Player* ob, Location loc);
		bool check_valid_location(Location loc);

		// Returns 0 for success add
		// 		   1 for adding to a tile with intersection to wall
		char add_object(Player* ob, Location loc);
		
		// Returns 0 for success removal
		// 		   1 for Object bitmap all 0
		// 		   2 for Object doesn't exist at a tile
		char remove_object(Player* ob);
};
#endif
