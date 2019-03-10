#pragma once

#include "common.hpp"
class ThunderString : Renderable
{
	static Texture texture;
public:
	ThunderString()
	{
	}

	ThunderString(vec2 position)
	{
		init(position);
	}

	bool init(vec2 position);

	void destroy();

	void update(float ms);

	void draw(const mat3 &projection);


protected:
	vec2 m_position; // Window coordinates
	vec2 end_position; //The end position of the thunderstring
	vec2 m_scale; // 1.f in each dimension. 1.f is as big as the associated texture
	float m_rotation; // in radians
	vec2 velocity;
	float initial_speed;
	
};

