#include "Object.h"

Object::Object(std::string const & name, Location loc, char type,
				Object* next) {
	this->name = name;
	this->loc = loc;
	this->type = type;
	this->next = next;
}

Object::~Object() {}

std::string Object::get_name() {
	return this->name;
}

Location Object::get_loc() {
	return this->loc;
}

char Object::get_type() {
	return this->type;
}

void Object::set_name(std::string const & name) {
	this->name = name;
}

void Object::set_loc(Location loc) {
	this->loc = loc;
}

void Object::set_type(char type) {
	this->type = type;
}

void Object::print() {
	std::cout << "Object Name: " << this->name << std::endl <<
				 "Type:		   " << this->type << std::endl <<
				 "Location:    (" << std::get<0>(this->loc) << ", " <<
				 					 std::get<1>(this->loc) << ")" << 
									 std::endl <<
				 "Next Object: " << next << std::endl;
}

