#pragma once

#include "common.hpp"
#include "sprite_sheet.hpp"
class EnemyPowerupWave : Renderable
{
	static SpriteSheet texture;
public:
	EnemyPowerupWave()
	{
	}

	EnemyPowerupWave(vec2 position, vec3 color)
	{
		init(position, color);
	}

	bool init(vec2 position, vec3 color);

	void destroy(bool reset);

	void update(float ms);

	void draw(const mat3 &projection);

	void setTextureLocs(int index);

	vec2 m_position; // Window coordinates

	vec3 custom_color;

protected:

	vec2 m_scale; // 1.f in each dimension. 1.f is as big as the associated texture
	float m_rotation; // in radians
	float animation_time;
	TexturedVertex texVertices[4];
	std::vector<float> texture_locs;
	bool first_time;
};

