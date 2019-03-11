// Header
#include "enemies.hpp"
#include "Thunder.h"
#include <cmath>
#include <algorithm>

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

void Enemies::take_damage(float damage, vec2 direction){
	hp = hp - 1.f * damage;
	if (hp <= 0.5f) {
		m_is_alive = false;
	}
	momentum.x = direction.x / 100.f * damage / 10.f * momentum_factor;
	momentum.y = direction.y / 100.f * damage / 10.f * momentum_factor;
}

void Enemies::take_damage(float damage) {
	hp = hp - 1.f * damage;
	if (hp <= 0.5f) {
		m_is_alive = false;
	}
}

bool Enemies::is_alive()const
{
	return m_is_alive;
}

bool Enemies::collide_with(Projectile &projectile)
{
	float dx = m_position.x - projectile.get_position().x;
	float dy = m_position.y - projectile.get_position().y;
	float d_sq = dx * dx + dy * dy;
	float other_r = std::max(projectile.get_bounding_box().x, projectile.get_bounding_box().y);
	float my_r = std::max(m_scale.x, m_scale.y);
	float r = std::max(other_r, my_r);
	r *= 1.f;
	if (d_sq < r * r)
		return true;
	return false;
}

bool Enemies::collide_with(Thunder &thunder)
{
	float dx = m_position.x - thunder.get_position().x;
	float dy = m_position.y - thunder.get_position().y;
	float d_sq = dx * dx + dy * dy;
	float other_r = thunder.get_radius();
	float my_r = std::max(m_scale.x, m_scale.y);
	float r = std::max(other_r, my_r);
	r *= 0.6f;
	if (d_sq < r * r)
		return true;
	return false;
}

void Enemies::set_speed(float speed)
{
	m_speed = speed;
}

float Enemies::get_speed()
{
	return m_speed;
}

void Enemies::set_stunded(bool hit)
{
	stunned = hit;
}