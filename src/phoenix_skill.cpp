#include "phoenix_skill.h"
#define _USE_MATH_DEFINES
#include <math.h>

void phoenix_skill::init()
{
	m_hp = 100.f;
	damage = 10.f;
	m_scale = {0.8f,0.8f};
	mp_cost = 14.f;
}

float phoenix_skill::create_phoenix(std::vector<phoenix*> &phoenix_list,vec2 hero_position)
{
	int max_phoenix = 3;
	int current_size = phoenix_list.size();
	float initial_angle = 2 * M_PI / max_phoenix;
	if (current_size > 0)
		initial_angle = phoenix_list[current_size - 1]->get_angle();
	float radius = 150.f;
	if (current_size < max_phoenix)
	{
		float angle = 2 * M_PI / max_phoenix + initial_angle;
		float dx = cos(angle) * radius;
		float dy = sin(angle) * radius;
		vec2 position = { hero_position.x + dx, hero_position.y + dy };
		phoenix* p = new phoenix(m_hp, damage, position, m_scale, angle);
		phoenix_list.emplace_back(p);
	}
	else
		return 0.f;
	return mp_cost;
}

bool phoenix_skill::level_up(int select)
{
	bool success = false;
	switch (select)
	{
	case LEVEL_UP_DAMAGE: // increase damage
		if (damage_level < 5)
		{
			damage *= 1.2f;
			mp_cost += 2.f;
			success = true;
			damage_level++;
		}
		break;
	case LEVEL_UP_EFFECT: //
		if (effect_level < 5)
		{
			m_hp *= 1.15f;
			mp_cost += 1.5f;
			success = true;
			effect_level++;
		}

		break;
	case LEVEL_UP_MANA_COST:
		if (mp_cost_level < 5)
		{
			mp_cost *= 0.8;
			success = true;
			mp_cost_level++;
		}
		break;
	default:
		break;
	}
	return success;
}