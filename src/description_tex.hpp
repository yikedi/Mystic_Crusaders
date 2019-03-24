#pragma once
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "common.hpp"

class Description_tex : public Renderable
{
	static Texture square_texture;
public:

	// Creates all the associated render resources and default transform
	bool init(vec2 screen);

	// Releases all the associated resources
	void destroy();

	void draw(const mat3& projection)override;

	void update_description(bool paused, int skill_num, vec2 screen); //call on world

	vec2 get_position() const;

	void get_texture(int loc);

private:
	vec2 m_scale;
	vec2 m_position;
	TexturedVertex vertices[4];
	
};

