#pragma once
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "common.hpp"


class Mapscreen : public Renderable
{
	static Texture map_screen;

public:
	// Creates all the associated render resources and default transform
	bool init(vec2 screen);	//vec2 screen

	// Releases all associated resources
	void destroy();

	// Renders
	void draw(const mat3& projection)override;
	void update(Mapscreen s);
	vec2 set_scale(float w, float h, vec2 screen);
	bool is_over();
	void set_is_over(bool over);

private:
	vec2 m_position; // Window coordinates
	vec2 m_scale; // 1.f in each dimension. 1.f is as big as the associated texture
	float m_rotation;
	int g_level;
	bool m_is_over;

};