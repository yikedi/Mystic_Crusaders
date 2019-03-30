#pragma once

#include "common.hpp"
#include "sprite_sheet.hpp"
class ThunderString : Renderable
{
	static SpriteSheet texture;
public:
	ThunderString()
	{
	}

	ThunderString(vec2 position, vec3 color)
	{
		init(position, color);
	}

	bool init(vec2 position, vec3 color);

	void destroy();

	void update(float ms);

	void draw(const mat3 &projection);

	void setTextureLocs(int index);


protected:

	vec2 m_position; // Window coordinates
	vec2 end_position; //The end position of the thunderstring
	vec2 m_scale; // 1.f in each dimension. 1.f is as big as the associated texture
	float m_rotation; // in radians
	vec2 velocity;
	float initial_speed;
	float animation_time;
	TexturedVertex texVertices[4];
	std::vector<float> texture_locs;
	bool first_time;
	vec3 custom_color;
};

