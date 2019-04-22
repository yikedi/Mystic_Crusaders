#pragma once
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "common.hpp"


class TutorialScreen : public Renderable
{
	static Texture tutorial_screen;

public:
	// Creates all the associated render resources and default transform
	bool init(vec2 screen);	//vec2 screen

	void destroy();

	// Releases all associated resources
	void destroy(bool reset);

	// Renders
	void draw(const mat3& projection)override;
	void update(bool tutorial_display, int page_num);
	void get_texture(int loc);
	vec2 set_scale(float w, float h, vec2 screen);

private:
	vec2 m_position; // Window coordinates
	vec2 m_scale; // 1.f in each dimension. 1.f is as big as the associated texture
	float m_rotation;
	TexturedVertex vertices[4];
};