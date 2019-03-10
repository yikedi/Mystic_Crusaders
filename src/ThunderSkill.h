#pragma once

#include "common.hpp"
#include "Thunder.h"

class ThunderSkill
{
public:
	
	void init();

	void level_up(int select);

	float drop_thunder(std::vector<Thunder*> &thunders, vec2 position);

	float get_mpcost();

protected:
	float mp_cost;
	float damage;
	vec2 scale;
	int level;
	float impactTime;
};



