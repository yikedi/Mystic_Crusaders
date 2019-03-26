#pragma once

#include "common.hpp"
#include "Skill.h"

class phoenix_skill : public Skill
{
public:

	void init();

	bool level_up(int select);

	//float drop_thunder(std::vector<Thunder*> &thunders, vec2 position);

	// phoneix create_phoenix()

	float get_mpcost();
};

