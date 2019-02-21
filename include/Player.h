#ifndef _player_h
#define _player_h

#include <iostream>
#include "Object.h"
#include "Map.h"
#include <cmath>
typedef std::pair<float, float> Exact_location;
class Map;
class Player : public Object {
	private:
		// Radius represents the size of the object
		int rad, hp, att, def, ms; // MS for movement speed (per second)
		int unit_id;
		// Exact location uses float, avoids accumulation of rounding
		Exact_location exact_loc;
		// Should be init as -1 and -1
		Location target_loc;

		// Binary 3x3 matrix represeting if object is in nearby tiles
		char map_rep[9];

	public:
		
		// Constructor/Destructor
		Player(std::string const &s, Location loc, 
					int rad, int hp, int att, int def);
		~Player();
		
		// Get Functions
		int get_hp();
		int get_exact_loc();
		int get_att();
		int get_def();
		int get_rad();
		int get_ms();

		// Returns shallow copy
		char* get_map_rep();
	
		// Set Functions
		void set_hp(int hp);
		void set_exact_loc(Exact_location eloc);
		void set_att(int att);
		void set_def(int def);
		void set_rad(int rad);
		void set_ms(int ms);
		void add_exact_loc(Exact_location eloc);
		void set_map_rep(char map_rep[9]);

		// Return 0 for unsuccessful, 1 for successful
		// Doesn't update exact location
		char move(Location loc, Map *m);
		
		// Updates location based on target location every
		// Seconds per Frame.
		char update_location(Map *m);

		// Helper Functions
		void print();
};

#endif
