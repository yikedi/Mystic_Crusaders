#include "Thunder.h"

float string_time = 300.f;
bool Thunder::init(vec2 position, float m_impactTime, float damage,vec2 m_scale, vec3 color, bool isFireRing)
{
	// Load shared texture
	if (!isFireRing){
		thunderString.init(position, color);
	}
	thunderBall.init(position, m_scale, color, isFireRing);
	m_isFireRing = isFireRing;
	impactTime = m_impactTime;
	elapsedTime = 0.f;
	m_position = position;
	m_damage = damage;
	return true;
}

vec2 Thunder::get_position()
{
	return m_position;
}

void Thunder::set_position(vec2 position)
{
	m_position = position;
}

void Thunder::update(float ms)
{
	elapsedTime += ms;
	if (m_isFireRing) {
		thunderBall.update(elapsedTime);
	} else {
		if (elapsedTime < string_time)
		{
			thunderString.update(elapsedTime);
		}
		else if (elapsedTime < impactTime)
		{
			thunderBall.update(elapsedTime);
		}
	}
}

void Thunder::draw(const mat3 &projection)
{
	if (m_isFireRing) {
		thunderBall.draw(projection);
	} else {
		if (elapsedTime < string_time)
		{
			thunderString.draw(projection);
		}
		else if (elapsedTime < impactTime)
		{
			thunderBall.draw(projection);
		}
	}
}

void Thunder::destroy()
{
	thunderString.destroy(true);
	thunderBall.destroy(true);
}

bool Thunder::can_remove()
{
	return elapsedTime > impactTime;
}

float Thunder::get_radius()
{
	return 0.6 * thunderBall.get_radius();
}

void Thunder::apply_effect(Enemies & e)
{
	e.take_damage(m_damage);
	e.set_stunded(true);
}

void Thunder::set_color(vec3 color)
{
	custom_color = color;
}