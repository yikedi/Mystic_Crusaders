#pragma once
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "common.hpp"

class Shop_item : public Renderable
{
	Texture item_texture;
public:

	// Creates all the associated render resources and default transform
	bool init(vec2 screen);
	void destroy();


	// Releases all the associated resources
	void destroy(bool reset);

	void draw(const mat3& projection)override;

	void update_item(bool shopping, int item_num); //call on world

	void get_texture(int loc);

	vec2 get_position() const;

private:
	vec2 m_scale;
	vec2 m_position;
	std::string element;

	TexturedVertex vertices[4];
	int m_light_up;
};

