#pragma once
#include "common.hpp"
#include "ThunderString.h"
#include "ThunderBall.h"
#include "enemies.hpp"

class Thunder: public Renderable
{

public:
	Thunder()
	{}

	Thunder(vec2 position, float impactTime = 3000.f, float damage = 40.f, vec2 scale = {1.0f,1.0f})
	{
		init(position, impactTime, damage,scale);
	}

	bool init(vec2 position, float impactTime, float damage, vec2 scale);

	void set_position(vec2 position);

	vec2 get_position();

	void update(float ms);

	void draw(const mat3 &projection);

	void destroy();

	float get_radius();

	bool can_remove();

	void apply_effect(Enemies & e);

	~Thunder() 
	{
		destroy();
	}

protected:
	ThunderBall thunderBall;
	ThunderString thunderString;
	float elapsedTime;
	float impactTime;
	vec2 m_position;
	float m_damage;
};

