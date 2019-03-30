#pragma once
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "common.hpp"
#include "ice_skill_tex.hpp"
#include "thunder_skill_tex.hpp"
#include "fire_skill_tex.hpp"
#include "skillup.hpp"
#include "description_tex.hpp"

class Skilltree : public Renderable
{
	
	static Texture screen_texture;


public:

	// Creates all the associated render resources and default transform
	bool init(vec2 screen, int element);

	// Releases all the associated resources
	void destroy();

	void draw(const mat3& projection)override;

	void update_skill(bool paused, int total, int used, vec3 ice_num, vec3 thunder_num, vec3 fire_num, int skill_num, vec2 screen);

	std::string get_element();

	bool inside(vec2 h, vec2 w, vec2 pos);

	bool level_position(vec2 mouse_pos);

	int icon_position(vec2 mouse_pos, std::string element);

	std::string element_position(vec2 mouse_pos);

private:
	Iceskilltex ices1;
	Iceskilltex ices2;
	Iceskilltex ices3;

	Thunderskilltex thunder1;
	Thunderskilltex thunder2;
	Thunderskilltex thunder3;

	Fireskilltex fire1;
	Fireskilltex fire2;
	Fireskilltex fire3;

	Skillup skillup;
	Description_tex description;
	vec2 m_scale;
	vec2 m_position;
	std::string front_element;
	std::string upper_element; 
	std::string lower_element;

	TexturedVertex vertices[4];
	TexturedVertex vertices0[4];
	TexturedVertex vertices2[4];

	int skillpoints;
	int used_skillpoints;
	int free_skillpoints;

};