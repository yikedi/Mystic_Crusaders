
#pragma once

#include "common.hpp"
#include "sprite_sheet.hpp"
#include "screen_button.hpp"

class Scrollable : public Renderable
{

	Texture scroll_texture;

public:

	bool init(vec2 position, vec2 screen, float increment);

	void destroy();

	void update();

	void draw(const mat3 &projection);

	bool check_position_for_ending();

	float m_color[3];
	float m_color_transparent[4];

private:

	vec2 m_position; // Window coordinates
	vec2 m_scale;
	float scroll_width;
	float scroll_height;
	bool m_is_in_use;
	float _increment;

};