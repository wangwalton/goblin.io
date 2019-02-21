#ifndef _grid_h
#define _grid_h

#include "Object.h"
#include "Player.h"
class Player;
typedef struct mon{
	Player *pl;
	struct mon *next;
} MoveableObjectNode;

class Grid {
	private:
		// 1 for all wall, 0 for not all wall
		char type;
		Object* wall_ob;
		MoveableObjectNode* pl;

	public:
		// Default, not all wall, nullptrs
		Grid();
		Grid(char type, Object* wall_ob);
		~Grid();

		// Get functions
		char get_type();
		Object* get_wall_ob();
		MoveableObjectNode* get_monp();

		// Set functions
		void set_type(char type);
		void set_wall_ob(Object* wall_ob);
		void set_monp(MoveableObjectNode* monp);
		
		bool check_grid_valid_location(Location loc);
		
		// Return 1 if object not found in grid, return 0 if successful
		char remove_ob_by_addr(Player* target);

		// Add to head
		void add_ob_by_addr(Player* target);
};

#endif
