#include "Thunder.h"


bool Thunder::init(vec2 position, float m_impactTime, float damage,vec2 m_scale)
{
	// Load shared texture
	thunderString.init(position);
	thunderBall.init(position, m_scale);
	impactTime = m_impactTime;
	elapsedTime = 0.f;
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
	if (elapsedTime < 200 )
	{
		thunderString.update(elapsedTime);
	}
	else if (elapsedTime < impactTime)
	{
		thunderBall.update(elapsedTime);
	}
}

void Thunder::draw(const mat3 &projection)
{
	if (elapsedTime < 200)
	{
		thunderString.draw(projection);
	}
	else if (elapsedTime < impactTime)
	{
		thunderBall.draw(projection);
	}
}

void Thunder::destroy()
{
	thunderString.destroy();
	thunderBall.destroy();
}

bool Thunder::can_remove()
{
	return elapsedTime > impactTime;
}