#include <iostream>
#include "Player.h"
#include "Object.h"
#include "Map.h"

int main() {
	Map m;
	Location x (1, 1);
	Player p1("Walton", x, 1,1,1);
	m.add_object(&p1, x);
	m.print_map();
	Location x2 (2,2);
	p1.move(x2, &m);
	m.print_map();
	return 1;
}
