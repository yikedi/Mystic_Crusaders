#pragma once

#include "common.hpp"
#include "projectile.h"
#include "fireball.h"
#include "enemy_01.hpp"
#include "enemy_02.hpp"
#include "hero.hpp"
#include "world.hpp"

class Enemy_01;
class Fish;

class UserInterface : public Renderable
{

	static Texture UserInterface_texture;

public:
	// Creates all the associated render resources and default transform
	//hero attributes
	float max_hp;
	float max_mp;
	float hp;
	float mp;
	//std::vector<skill> skill_list;

	float m_color[3];
	bool advanced;
	vec2 m_screen;
	bool init(vec2 screen);

	// Releases all associated resources
	void destroy();

	// Update salmon position based on direction
	// ms represents the number of milliseconds elapsed from the previous update() call
	void update(float ms);

	// Renders the salmon
	void draw(const mat3& projection)override;

	// Returns the current salmon position
	vec2 get_position()const;

	// Moves the salmon's position by the specified offset
	void move(vec2 off);

	// Set salmon rotation in radians
	void set_rotation(float radians);

	// Called when the salmon collides with a fish, starts lighting up the salmon
	void light_up();

	void set_color(vec3 color);

	//hero functions
	void change_hp(float d_hp);
	void change_mp(float d_mp);

private:
	World m_world;
	Hero m_hero;
	double w;
	double h;
	float m_light_up_countdown_ms; // Used to keep track for how long the salmon should be lit up
	bool m_is_alive; // True if the salmon is alive
	vec2 m_position; // Window coordinates
	vec2 m_scale; // 1.f in each dimension. 1.f is as big as the associated texture
	float m_rotation; // in radians
	size_t m_num_indices; // passed to glDrawElements

	//add salmon speed
	vec2 m_direction;
	int m_light_up;
};
