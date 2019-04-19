#pragma once

#include "common.hpp"
#include "sprite_sheet.hpp"
#include "screen_button.hpp"
#include "time.h"

class Scrollable : public Renderable
{

	Texture scroll_texture;

public:

	bool init(vec2 position, int w, int h, float time);

	void destroy();

	void update(int skill, float zoom);

	void setTime(float curr_time);

	void setTextureLocs(float time);

	void draw(const mat3 &projection);

	void set_position(vec2 position);

	float m_color[3];
	float m_color_transparent[4];

private:

	clock_t timePassed;

	TexturedVertex texVertices[4];

	std::vector<float> texture_locs;

	float total_time;
	float current_time;
	vec2 m_position; // Window coordinates
	vec2 m_scale;
	float zoom_factor;
	float width;
	float height;
	float opacity = 1.f;
};