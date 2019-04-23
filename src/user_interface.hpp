#pragma once

#include "common.hpp"

class UserInterface : public Renderable
{

	static Texture UserInterface_texture;

public:
	// Creates all the associated render resources and default transform
	//hero attributes
	float max_hp;
	float max_mp;
	float hp;
	float mp;
	int max_exp;
	int cur_exp;
	//std::vector<skill> skill_list;

	float m_color[3];
	bool advanced;
	vec2 m_screen;
	bool init(vec2 screen, float max_hp);

	// Releases all associated resources
	void destroy();

	// Update salmon position based on direction
	// ms represents the number of milliseconds elapsed from the previous update() call
	void update(vec2 hp_mp, vec2 exp, float zoom_factor, float max_hp);

	// Renders the salmon
	void draw(const mat3& projection)override;

	// Set rotation, should there ever be the need for it
	void set_rotation(float radians);

	void light_up();

	void set_color(vec3 color);

	//hero functions
	void change_hp(float d_hp);
	void change_mp(float d_mp);
	void set_position(vec2 position, int sh, int offset);

private:
	
	double w;
	double h;
	float zoom_factor;
	float m_light_up_countdown_ms;
	bool m_is_alive;
	vec2 m_position; // Window coordinates
	vec2 m_scale; // 1.f in each dimension. 1.f is as big as the associated texture
	float m_rotation; // in radians
	size_t m_num_indices; // passed to glDrawElements

	int m_light_up;
};
