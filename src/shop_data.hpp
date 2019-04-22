#pragma once
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "common.hpp"

class Shop_data : public Renderable
{
	Texture number_texture;
public:

	// Creates all the associated render resources and default transform
	bool init(vec2 screen, int which);
	void destroy();

	void draw(const mat3& projection)override;

	void update_numbers(bool shopping, int number, int item_num);

	void update_numbers(bool shopping, int number);

	void get_texture(int loc);

	vec2 get_position() const;

private:
	vec2 m_scale;
	vec2 m_position;

	float zoom_factor;

	std::string element;

	TexturedVertex vertices[4];
};

