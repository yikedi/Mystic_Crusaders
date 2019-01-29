#pragma once

#include "common.hpp"
#include "enemies.hpp"

// Salmon enemy
class Enemy : public Renderable, public Enemies
{
	// Shared between all enemys, no need to load one for each instance
	static Texture enemy_texture;

public:

	// Releases all the associated resources
	void destroy() override;

	// Creates all the associated render resources and default transform
	bool init() override;

	// Update enemy due to current
	// ms represents the number of milliseconds elapsed from the previous update() call
	void update(float ms, vec2 target_pos) override;

	// Renders the salmon
	// projection is the 2D orthographic projection matrix
	void draw(const mat3& projection)override;

	void attack() override;
};