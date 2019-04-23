#pragma once

#include "common.hpp"

class Story : public Renderable
{

	static Texture story_texture;

public:

	bool init(vec2 screen);

	void destroy();

	void update();

	void draw(const mat3 &projection);

	float m_color[3];
	float m_color_transparent[4];

private:

	vec2 m_position; // Window coordinates
	vec2 m_scale; // 1.f in each dimension. 1.f is as big as the associated texture
	float m_rotation; // in radians

	float opacity = 1.f;
};