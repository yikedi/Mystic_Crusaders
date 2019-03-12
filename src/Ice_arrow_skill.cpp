//
// Created by douglas on 19/2/22.
//

#include "Ice_arrow_skill.h"
#include "Ice_arrow.h"

void Ice_arrow_skill::init()
{
    mp_cost = 2;
    damage = 20.f;
    damage_level = 0;
	effect_level = 0;
	mp_cost_level = 0;
    num_arrows = 3;
    shoot_range_angle = (float) M_PI * 15 /180;
}

void Ice_arrow_skill::set_range_angle(float radius) {shoot_range_angle = radius;}

float Ice_arrow_skill::shoot_ice_arrow(std::vector<Projectile *> &hero_projectiles, float radius, vec2 position)
{
    float d_angle = shoot_range_angle/(num_arrows-1);
    float start_angle = radius - shoot_range_angle/2.f;
    for (int i= 0; i<num_arrows; i++)
    {
        float current_angle = start_angle + d_angle * i;
        Ice_arrow* ice_arrow = new Ice_arrow(current_angle, 400.f, damage);
        ice_arrow->set_position(position);
        hero_projectiles.emplace_back(ice_arrow);
    }
    return mp_cost;

}

bool Ice_arrow_skill::level_up(int select)
{
/*
    if (level < 4)
    {
        level++;
        damage += 10;
        num_arrows += 1;
        shoot_range_angle += (float) M_PI * 7.5 /180;
        mp_cost += 1;
    }
*/
	bool success = false;
	switch (select)
	{
	case LEVEL_UP_DAMAGE:
		if (damage_level < 4)
		{
			damage += 10;
			mp_cost += 1;
			success = true;
		}
		break;
	case LEVEL_UP_EFFECT:
		if (effect_level < 4)
		{
			num_arrows += 1;
			shoot_range_angle += (float)M_PI * 7.5 / 180;
			mp_cost += 1;
			success = true;
		}
		break;
	case LEVEL_UP_MANA_COST:
		if (mp_cost_level < 4)
		{
			mp_cost *= 0.8;
			success = true;
		}
		break;
	default:
		break;
	}
	return success;
}