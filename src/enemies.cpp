// Header
#include "enemies.hpp"

#include <cmath>

Texture Enemies::enemy_texture;

vec2 Enemies::get_position()const
{
	return m_position;
}

void Enemies::set_position(vec2 position)
{
	m_position = position;
}

void Enemies::set_facing(int rotation_value) {
	m_face_left_or_right = rotation_value;
}

void Enemies::set_rotation(float rotation_value) {
	m_rotation = rotation_value;
}

// Returns the local bounding coordinates scaled by the current size of the enemy
vec2 Enemies::get_bounding_box()const
{
	// fabs is to avoid negative scale due to the facing direction
	return { std::fabs(m_scale.x) * enemy_texture.width, std::fabs(m_scale.y) * enemy_texture.height };
}

void Enemies::setRandMovementTime(clock_t c){
	randMovementTime = c;
}

bool Enemies::checkIfCanChangeDirectionOfMove(clock_t currentClock){
	if ((double)(currentClock - randMovementTime) > randMovementCooldown) {
		return true;
	}
	return false;
}