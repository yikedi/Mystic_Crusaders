#pragma once

#include "common.hpp"
#include "Thunder.h"
#include "Skill.h"


class ThunderSkill : public Skill
{
public:
	
	void init();

	bool level_up(int select);

	float drop_thunder(std::vector<Thunder*> &thunders, vec2 position);

	float get_mpcost();

protected:
	vec2 scale;
	float impactTime;
};



