#pragma once
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "common.hpp"

class Numbers : public Renderable
{
	Texture number_texture;
public:

	// Creates all the associated render resources and default transform
	bool init(vec2 screen, int which); 

	// Releases all the associated resources
	void destroy();

	void draw(const mat3& projection)override;

	void update_numbers(bool start_is_over, float number, float world_zoom, vec2 hero_pos);

	void get_texture(int loc);

	vec2 get_position() const;

	void set_position(vec2 position, int sh, int offset);

private:
	vec2 m_scale;
	vec2 m_position;

	float zoom_factor;

	std::string element;

	TexturedVertex vertices[4];

	int m_light_up;
};

