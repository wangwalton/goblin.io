#include <iostream>
#include "Grid.h"
#include "Player.h"
#include "Object.h"
#include "Map.h"
#include <chrono>
#include <thread>

int main() {
	std::chrono::seconds a(1);
	Map m;
	Location x (5, 5);

	// Constructing player
	Player p1("Walton", x, 1,1,1, 1);
	p1.set_rad(3);
	std::cout << p1.next << std::endl;
	m.add_object(&p1, x);
	m.print_map();
	Location x2 (10, 10);

	// Location Update
	std::this_thread::sleep_for(a);
	p1.move(x2, &m);
	m.print_map();
	
	
	return 1;

}
