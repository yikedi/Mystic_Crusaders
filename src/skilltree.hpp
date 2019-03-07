#pragma once
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "common.hpp"
#include "ice_skill.hpp"
#include "skillup.hpp"

class Skilltree : public Renderable
{
	
	static Texture screen_texture;


public:

	// Creates all the associated render resources and default transform
	bool init(vec2 screen);

	// Releases all the associated resources
	void destroy();

	void draw(const mat3& projection)override;

	void update_skill(bool paused, int total, int used, vec3 ice_num); //call on world

	void get_texture(int loc);

	std::string get_element();

	int get_skillpoints(int m_points);

	void update_skillpoints(int level);

	vec2 set_scale(float w, float h, vec2 screen);

	bool in_position(vec2 mouse_pos, vec2 screen, int used);

	void reset();

private:
	Iceskill ices;
	Skillup skillup;
	vec2 m_scale;
	vec2 m_position;
	// skill element
	std::string element;

	TexturedVertex vertices[4];
	TexturedVertex vertices0[4];
	TexturedVertex vertices2[4];

	// level and degree for skills
	int level;
	int degree;
	int times;

	int skillpoints;
	int used_skillpoints;
	int free_skillpoints;
	int conversion = 10;


	// incre # of amo
	// decre mp cost
	// incre damage

	// vec3 skill{fire, 2,2, times}
};