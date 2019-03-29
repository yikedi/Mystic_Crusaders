#pragma once


#include "common.hpp"
#include "projectile.h"
#include "fireball.h"
#include "enemies.hpp"
#include "enemy_laser.h"
#include "time.h"

class Enemy_01;
class Fish;

class Vine : public Renderable
{
	static SpriteSheet vine_texture;

public:
	// Creates all the associated render resources and default transform
	//hero attributes

	//std::vector<skill> skill_list;

	vec2 m_screen;
	bool init(vec2 screen);

	// Releases all associated resources
	void destroy();

	void update(float ms);

	void draw(const mat3& projection)override;

	// Returns the current salmon position
	vec2 get_position()const;

	void set_position(vec2 position);

	void setTextureLocs(int index);

	vec2 get_bounding_box();

	float get_damage() const;


private:
	double w;
	double h;
	float zoom_factor;
	vec2 m_position; // Window coordinates
	vec2 m_scale; // 1.f in each dimension. 1.f is as big as the associated texture
	float m_rotation; // in radians
	size_t m_num_indices; // passed to glDrawElements

	//add salmon speed
	vec2 m_direction;
	int m_light_up;
	vec3 m_color;

	//spritesheet
	clock_t timePassed;

	float variation;

	TexturedVertex texVertices[4];

	std::vector<float> texture_locs;

	float m_animTime = 0.0f;

	int numTiles;

	bool m_is_alive;

	float damage;



};
