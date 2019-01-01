#ifndef _grid_h
#define _grid_h

#include "Object.h"

class Grid {
	private:
		// 1 for all wall, 0 for not all wall
		char type;
		Object* wall_ob;
		Object* ob;

	public:
		// Default, not all wall, nullptrs
		Grid();
		Grid(char type, Object* wall_ob);
		~Grid();

		// Get functions
		char get_type();
		Object* get_wall_ob();
		Object* get_ob();

		// Set functions
		void set_type(char type);
		void set_wall_ob(Object* wall_ob);
		void set_ob(Object* ob);
		
		bool check_grid_valid_location(Location loc);

		char remove_ob_by_addr(Object* target);

		// Add to head
		void add_ob_by_addr(Object* target);
};

#endif
