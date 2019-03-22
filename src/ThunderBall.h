#pragma once
#include "common.hpp"
#include "sprite_sheet.hpp"
class ThunderBall : Renderable
{

	static SpriteSheet texture;
public:
	ThunderBall()
	{
	}

	bool init(vec2 position, vec2 scale = {2.5f,2.5f}, vec3 color={1.f,1.f,1.f});

	void set_position(vec2 position);

	vec2 get_position();

	void update(float ms);

	void draw(const mat3 &projection);

	void destroy();

	vec2 get_bounding_box();

	float get_radius();

	void setTextureLocs(int index);

protected:
	vec2 m_position; // Window coordinates
	vec2 m_scale; // 1.f in each dimension. 1.f is as big as the associated texture
	float m_rotation; // in radians
	float elapsedTime;
	float animation_time;
	TexturedVertex texVertices[4];
	std::vector<float> texture_locs;
	bool first_time;
	vec3 custom_color;
};

