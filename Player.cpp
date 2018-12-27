#include "Player.h"

int Player::getX() {
    return this->x;
}

int Player::getY() {
    return this->y;
}
int Player::getHp() {
    return this->hp;
}
int Player::getAtt() {
    return this->att;
}
int Player::getDef() {
    return this->def;
}

std::string Player::get_name() {
	return this->name;
}

void Player::setX(int x) {
    this->x = x;
}

void Player::setY(int y) {
    this->y = y;
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

void Player::set_name(std::string const &s) {
	this->name = s;
}

void Player::print() {
	std::cout << "Player ID: " << this->name << std::endl
			  << "Location:  (" << this->x << ", " << this->y 
			  					<< ")" << std::endl
			  << "Hitpoints: " << this->hp << std::endl
			  << "Attack:    " << this->att << std::endl
			  << "Defense:   " << this->def << std::endl;
}

Player::Player(std::string const &s,
			   int x, int y, int hp, int att, int def ) {
    this->x = x;
	this->y = y;
	this->hp = hp;
	this->att = att;
	this->def = def;
	this->name = s;
}


Player::~Player() {}
