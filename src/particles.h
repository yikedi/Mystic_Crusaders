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

    void set_lifetime(float t);

    float get_lifetime();

    void set_position(vec2 pos);

    vec2 get_position();

    void set_scale(float scale);

    void set_velocity(vec2 v);

    vec2 get_velocity();

    void set_color_green(float green);

    float get_color_green();

	void destroy();

protected:

	float life_time;
	float elapsed_time;
    vec3 m_color;
	vec2 m_position;
	vec2 m_scale;
	vec2 velocity;
	bool can_remove;


};

