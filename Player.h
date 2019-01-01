#ifndef _player_h
#define _player_h

#include <iostream>
#include "Object.h"
#include "Map.h"

class Player : public Object {
	private:
		int hp, att, def;
	public:
		
		// Constructor/Destructor
		Player(std::string const &s, Location loc, 
					int hp, int att, int def);
		~Player();
		
		// Get Functions
		int getHp();
		int getAtt();
		int getDef();
	
		// Set Functions
		void setHp(int hp);
		void setAtt(int att);
		void setDef(int def);
		
		// Return 0 for unsuccessful, 1 for successful
		char move(Location loc, Map* m);

		// Helper Functions
		void print();
};

#endif
