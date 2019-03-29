#pragma once
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "common.hpp"
class Bar_description : public Renderable
{
	static Texture hme_texture;
public:

	// Creates all the associated render resources and default transform
	bool init(vec2 screen);


	// Releases all the associated resources
	void destroy();

	void draw(const mat3& projection)override;

	void update_hme(vec2 hero_pos, float world_zoom, vec2 screen);

	void set_position(vec2 position, int sh, int offset);

	void update_hme(vec2 hero_position, float world_zoom);

private:
	vec2 m_scale;
	vec2 m_position;
	TexturedVertex vertices[4];
	float zoom_factor;
};

