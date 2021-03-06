#ifndef _object_h
#define _object_h

#include <iostream>

typedef std::pair<int, int> Location;

class Object {
	private:
		std::string name;
		Location loc;
		char type;

	public:
		Object* next;
		
		// Constructor / Destructors
		Object(std::string const & name, Location loc, char type,
				Object* next);
		~Object();

		// Get functions
		std::string get_name();
		Location get_loc();
		char get_type();

		// Set functions
		void set_name(std::string const & name);
		void set_loc(Location loc);
		void set_type(char type);

		// Misc functions
		void print();
};

#endif
