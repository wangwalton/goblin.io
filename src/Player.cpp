#include "include/Player.h"

Player::Player(std::string const &s, Location loc,
			   int rad, int hp, int att, int def) : Object(s,
			   		loc, 0, nullptr) {
	this->rad = rad;
	this->hp = hp;
	this->att = att;
	this->def = def;
}

Player::~Player() {}

// Get functions
int Player::get_hp() {
    return this->hp;
}

int Player::get_att() {
    return this->att;
}
int Player::get_def() {
    return this->def;
}

int Player::get_rad() {
	return this->rad;
}

int Player::get_ms() {
	return this->ms;
}

char* Player::get_map_rep() {
	return this->map_rep;
}

// Set Functions
void Player::set_hp(int hp) {
    this->hp = hp;
}

void Player::set_att(int att) {
    this->att = att;
}

void Player::set_def(int def) {
    this->def = def;
}

void Player::set_rad(int rad) {
	this->rad = rad;
}

void Player::set_ms(int ms) {
	this->ms = ms;
}

void Player::set_map_rep(char map_rep[9]) {
	memcpy(this->map_rep, map_rep, 9);
}

// Doesn't update exact location
// m (map) will be a global variable for now, not sure why its giving the
// error of unknown type map, i included map.h earlier
char Player::move(Location loc, Map *m) {
	if (!m->check_valid_location(loc)) return 0;
	m->move_object(this, loc);
	this->set_loc(loc);
	return 1;
}

void Player::add_exact_loc(Exact_location eloc) {
	float x = std::get<0>(eloc) + std::get<0>(this->exact_loc);
	float y = std::get<1>(eloc) + std::get<1>(this->exact_loc);
	this->exact_loc = std::make_pair(x, y);
	return;
}

char Player::update_location(Map *m) {

	// -1 represents unit doesn't have a target location
	if (std::get<0>(this->target_loc) != -1) {
		float df_x = (std::get<0>(this->target_loc) - std::get<0>(this->exact_loc));
		float df_y = (std::get<1>(this->target_loc) - std::get<1>(this->exact_loc));

		float d_mag = sqrt(df_x * df_x + df_y * df_y);
		
		// Normalized delta position
		float dn_x = df_x / d_mag;
		float dn_y = df_y / d_mag;

		// Delta distance
		float d_x = dn_x * this->ms * SECONDS_PER_UPDATE;
		float d_y = dn_y * this->ms * SECONDS_PER_UPDATE;

		// Updating exact location
		Exact_location df (d_x, d_y);
		add_exact_loc(df);
		
		// Distance in integer form
		Location di (round(std::get<0>(this->exact_loc)),
					 round(std::get<1>(this->exact_loc)));
		
		// Updates rounded loc and modifies map structure
		this->move(di, m);
	}
	return 0;
	// Later implement targeting unit
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

