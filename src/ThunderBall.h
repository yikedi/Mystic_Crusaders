#pragma once
#include "common.hpp"
class ThunderBall : Renderable
{

	static Texture texture;
public:
	ThunderBall()
	{
	}

	bool init(vec2 position, vec2 scale = {1.f,1.f});

	void set_position(vec2 position);

	vec2 get_position();

	void update(float ms);

	void draw(const mat3 &projection);

	void destroy();

	vec2 get_bounding_box();

	float get_radius();

protected:
	vec2 m_position; // Window coordinates
	vec2 m_scale; // 1.f in each dimension. 1.f is as big as the associated texture
	float m_rotation; // in radians
	float elapsedTime;
};

