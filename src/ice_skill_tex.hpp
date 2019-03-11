#pragma once
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "common.hpp"

class Iceskilltex : public Renderable
{
	Texture ice_texture;
public:

	// Creates all the associated render resources and default transform
	bool init(vec2 screen, int skill_num);  //,std::string filename

	// Releases all the associated resources
	void destroy();

	void draw(const mat3& projection)override;

	void update_ice(bool paused, float degree); //call on world

	void get_texture(int loc);

	vec2 get_position() const;

	bool in_position(vec2 mouse_pos, vec2 screen, int used);

	void light_up();
	void blue_up();
	void reset();

private:
	vec2 m_scale;
	vec2 m_position;
	// skill element
	std::string element;

	TexturedVertex vertices[4];

	// level and degree for skills
	int level;
	int degree;
	int times;


	int m_light_up;
};

