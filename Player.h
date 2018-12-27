#ifndef _player_h
#define _player_h
#include <iostream>
class Player {
	private:
		int x, y, hp, att, def;
		std::string name;
	public:
		
		// Constructor/Destructor
		Player(std::string const &s,
			   int x, int y, int hp, int att, int def);
		~Player();
		
		// Get Functions
		int getX();
		int getY();
		int getHp();
		int getAtt();
		int getDef();
		std::string get_name();
	
		// Set Functions
		void setX(int x);
		void setY(int y);
		void setHp(int hp);
		void setAtt(int att);
		void setDef(int def);
		void set_name(std::string const &s);
		
		// Helper Functions
		void print();
};

#endif
