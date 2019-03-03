#pragma once
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "common.hpp"

class skilltree : public Renderable
{
	static Texture tree_texture;
	static Texture skill_texture1_1;
	static Texture skill_texture1_2;
	static Texture skill_texture1_3;
	static Texture skill_texture2_1;

public:

	// Creates all the associated render resources and default transform
	bool init(int level, int score);

	// Releases all the associated resources
	void destroy();

	void draw(const mat3& projection)override;

	void update_skill(std::string element, int level); //call on world

	bool is_paused();

	//take score - 
	void get_points(int usedpoints, int score);

	void update_points(int level);

private:
	vec2 mouse_pos;
	bool paused;
	int points;
	int used_points;
	int conversion;


	// incre # of amo
	// decre mp cost
	// incre damage

	// vec3 skill{fire, 2,2, times}
};