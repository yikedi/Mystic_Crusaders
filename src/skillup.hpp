#pragma once
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "common.hpp"

class Skillup : public Renderable
{
	static Texture level_texture;
public:

	// Creates all the associated render resources and default transform
	bool init(vec2 screen);

	// Releases all the associated resources
	void destroy();

	void draw(const mat3& projection)override;

	void update_leveltex(bool paused, int freepoints); //call on world

	void get_texture(int loc);

	bool in_position(vec2 mouse_pos, vec2 screen, int used);

	vec2 set_scale(float w, float h, vec2 screen);

	void reset();

private:
	vec2 m_scale;
	vec2 m_position;
	TexturedVertex vertices[4];

};

