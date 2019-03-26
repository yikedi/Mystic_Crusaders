#pragma once
#include "common.hpp"

class particles : public Renderable
{

	static Texture texture;

public:
	particles() {};
	particles(float lifetime, float scale, vec2 position, vec2 initial_velocity);
	~particles();

	bool init(float lifetime, float scale, vec2 position, vec2 initial_velocity);

	void update(float ms, vec2 source_position);

	void draw(const mat3 &projection);

	void destroy();

protected:

	float life_time;
	float elapsed_time;
	vec2 m_position;
	vec2 m_scale;
	vec2 velocity;
	bool can_remove;


};

