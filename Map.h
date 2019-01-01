#ifndef _map_h
#define _map_h

#include "Grid.h"
#include <iostream>

// Map constants
#define MAP_WIDTH 50
#define MAP_HEIGHT 25
#define REP_FACTOR 5
#define WALL '@'
#define SPACE ' '
#define PLAYER 'P'
#define MONSTER 'M'
#define MONSTER_TYPE 1
#define PLAYER_TYPE 0

class Map {
	public:
		// Right now only checks objects
		Grid map[MAP_HEIGHT / REP_FACTOR][MAP_WIDTH / REP_FACTOR];
		// Short cut
		char has_walls = 0;
		void print_map();
		
		// Simply moves an object within map, doesn't update internal
		// Object location or perform checking
		char move_object(Object* ob, Location loc);
		bool check_valid_location(Location loc);
		char add_object(Object* ob, Location loc);
};
#endif
