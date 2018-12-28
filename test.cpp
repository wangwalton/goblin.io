#include <iostream>
#include "Player.h"
#include "Object.h"

int main() {
	Location x (1, 1);
	Player p1("Walton", x, 1,1,1);
	p1.print();
	return 1;
}
