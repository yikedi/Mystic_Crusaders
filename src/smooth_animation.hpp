#pragma once

#include "common.hpp"
#include "sprite_sheet.hpp"
#include "screen_button.hpp"
#include "time.h"

class Story : public Renderable
{

	static SpriteSheet skill_texture;

public:

	bool init(vec2 position);

	void destroy();

	void update(int skill, float zoom);

	void transitionHelper();

	void setTextureLocs(int index);

	void draw(const mat3 &projection);

	void set_position(vec2 position);

	float m_color[3];
	float m_color_transparent[4];

private:

	clock_t timePassed;

	TexturedVertex texVertices[4];

	std::vector<float> texture_locs;
};