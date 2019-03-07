#pragma once
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "common.hpp"

class Iceskill : public Renderable
{
	static Texture ice_texture;
public:

	// Creates all the associated render resources and default transform
	bool init(vec2 screen);

	// Releases all the associated resources
	void destroy();

	void draw(const mat3& projection)override;

	void update_ice(bool paused, vec3 ice_num); //call on world

	void get_texture(int loc);

	vec2 set_scale(float w, float h, vec2 screen);

	void reset();

private:
	vec2 m_scale;
	vec2 m_position;
	// skill element
	std::string element;

	TexturedVertex vertices[4];

	// level and degree for skills
	int level;
	int degree;
	int times;
};

