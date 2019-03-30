#pragma once

#include "common.hpp"
#include "sprite_sheet.hpp"
#include "time.h"

// actual used includes
#include <cmath>
#include <string>

enum CurrentSkillState { ICEBLADES, PHOENIX, FIRESTORM };

class SkillSwitch: public Renderable 
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

	CurrentSkillState m_skillState;

	int tilesLeftToTransition;

	bool m_is_in_use;

	vec2 m_position;

	vec2 m_scale;

	float m_animTime = 0.f;

	bool movePositiveDirection;

	int prevSkill;

	int transitionToSkill;

	bool hasTransitionStarted;

	int currIndex;

	float zoom_factor;

};