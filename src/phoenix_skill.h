#pragma once

#include "common.hpp"
#include "Skill.h"
#include "phoenix.h"

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

class phoenix_skill : public Skill
{
public:

	void init();

	bool level_up(int select);

	//float drop_thunder(std::vector<Thunder*> &thunders, vec2 position);

	float create_phoenix(std::vector<phoenix*> &phoenix_list,vec2 position, Mix_Chunk* m_phoenix_sound);

protected:
	float m_hp;
	vec2 m_scale;

};

