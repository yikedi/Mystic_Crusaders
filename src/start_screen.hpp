#pragma once
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "common.hpp"


class Startscreen : public Renderable
{
	static Texture start_screen;

public:
	// Creates all the associated render resources and default transform
	bool init(vec2 screen);	//vec2 screen

	// Releases all associated resources
	void destroy();

	// Renders
	void draw(const mat3& projection)override;
	void update(Startscreen s);
	vec2 set_scale(float w, float h, vec2 screen);
	bool is_over();

private:
	vec2 m_position; // Window coordinates
	vec2 m_scale; // 1.f in each dimension. 1.f is as big as the associated texture
	float m_rotation;
	int g_level;
	bool s_is_over;
	
};