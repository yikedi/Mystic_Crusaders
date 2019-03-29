#pragma once
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "common.hpp"
#include "numbers.hpp"

class In_game : public Renderable
{
	Texture basic_texture;

public:

	// Creates all the associated render resources and default transform
	bool init(vec2 screen);

	// Releases all the associated resources
	void destroy();

	void draw(const mat3& projection)override;

	void update_ingame(bool start_is_over, vec3 level_num, vec3 kill_num, vec2 screen, vec2 hero_pos, float world_zoom);

	void set_position(vec2 position, int sh, int offset);

private:
	float zoom_factor;
	vec2 m_scale;
	vec2 m_position;
	vec2 hero_pos;
	Numbers num11;
	Numbers num12;
	Numbers num13;
	Numbers num21;
	Numbers num22;
	Numbers num23;

	vec2 size;
	int osx;
	int osh;

};

