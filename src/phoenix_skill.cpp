#include "phoenix_skill.h"
#define _USE_MATH_DEFINES
#include <math.h>

void phoenix_skill::init()
{
	m_hp = 150.f;
	damage = 20.f;
	m_scale = {1.f,1.f};
	mp_cost = 25.f;
}

float phoenix_skill::create_phoenix(std::vector<phoenix*> &phoenix_list,vec2 hero_position)
{
	int max_phoenix = 3;
	int current_size = phoenix_list.size();
	float initial_angle = 2 * M_PI / max_phoenix;
	float radius = 100.f;
	if (current_size < max_phoenix)
	{
		float angle = angle = current_size * 2 *M_PI / max_phoenix + initial_angle;
		
		float dx = cos(angle) * radius;
		float dy = sin(angle) * radius;
		vec2 position = {hero_position.x + dx, hero_position.y + dy};
		phoenix* p = new phoenix(m_hp, damage, position, m_scale,angle);
		phoenix_list.emplace_back(p);
	}
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
			damage += 2.f;
			mp_cost += 2.f;
			success = true;
			damage_level++;
		}
		break;
	case LEVEL_UP_EFFECT: //
		if (effect_level < 5)
		{
			m_scale = { m_scale.x * 1.15f , m_scale.y * 1.15f };
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