#include "Player.h"

Player::Player(std::string const &s, Location loc,
			   int hp, int att, int def) : Object(s,
			   		loc, 0, nullptr) {
	this->hp = hp;
	this->att = att;
	this->def = def;
}

Player::~Player() {}
int Player::getHp() {
    return this->hp;
}

int Player::getAtt() {
    return this->att;
}
int Player::getDef() {
    return this->def;
}

void Player::setHp(int hp) {
    this->hp = hp;
}

void Player::setAtt(int att) {
    this->att = att;
}

void Player::setDef(int def) {
    this->def = def;
}
char Player::move(Location loc, Map *m) {
	if (!m->check_valid_location(loc)) return 0;
	m->move_object(this, loc);
	this->set_loc(loc);
	return 1;
}

void Player::print() {
	Location loca = this->get_loc();

	std::cout << "Player ID: " << this->get_name() << std::endl <<
			     "Location : (" << std::get<0>(loca) << 
				 		", " << std::get<1>(loca) << ")" << std::endl
			  << "Hitpoints: " << this->hp << std::endl
			  << "Attack:    " << this->att << std::endl
			  << "Defense:   " << this->def << std::endl;
}

