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

	Thunder(vec2 position, float impactTime = 3000.f, float damage = 40.f, vec2 scale = {1.0f,1.0f}, vec3 color={1.f,1.f,1.f}, bool isFireRing = false)
	{
		init(position, impactTime, damage, scale, color, isFireRing);
	}

	bool init(vec2 position, float impactTime, float damage, vec2 scale, vec3 color = {1.f,1.f,1.f}, bool isFireRing = false);

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

	void set_color(vec3 color);

protected:
	ThunderBall thunderBall;
	ThunderString thunderString;
	float elapsedTime;
	float impactTime;
	vec2 m_position;
	float m_damage;
	vec3 custom_color;
	bool m_isFireRing;
};

