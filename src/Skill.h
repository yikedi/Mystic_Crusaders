#pragma once
constexpr auto LEVEL_UP_DAMAGE = 0;
constexpr auto LEVEL_UP_EFFECT = 1;
constexpr auto LEVEL_UP_MANA_COST = 2;

class Skill
{
public:
	virtual void init() = 0;

	virtual bool level_up(int select) = 0;

	virtual float get_mpcost();

	void set_mp_cost(float cost);

	void set_damage(float damage);

	float get_damage();

protected:
	float mp_cost;
	float damage;
	int damage_level;
	int effect_level;
	int mp_cost_level;
};

