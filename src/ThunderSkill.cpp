#include "ThunderSkill.h"

void ThunderSkill::init()
{
	mp_cost = 10;
	damage = 10;
	level = 0;
	impactTime = 3000;
	scale = { 0.3f,0.3f };
}
void ThunderSkill::level_up(int select)
{
	level += 1;
	switch (select) 
	{
		case 0: // increase damage
			damage = damage + 10;
			break;
		case 1: // 
			scale = { scale.x * 1.1f + scale.y * 1.1f };
			break;
		case 2:
			impactTime += 1000;
			break;
		default:
			break;
	}

}

float ThunderSkill::get_mpcost()
{
	return mp_cost;
}

float ThunderSkill::drop_thunder(std::vector<Thunder*> &thunders, vec2 position)
{
	Thunder* t = new Thunder(position, impactTime,damage,scale);
	thunders.emplace_back(t);
	return mp_cost;
}
