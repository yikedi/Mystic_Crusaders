#pragma once

#include "common.hpp"
#include "enemies.hpp"
#include "enemy_laser.h"
#include "projectile.h"
#include "time.h"

// Salmon enemy
class Enemy_02 :public Enemies
{
	// Shared between all enemys, no need to load one for each instance
	static SpriteSheet enemy_texture;

public:

	// Releases all the associated resources
	void destroy(bool reset) override;

	// Creates all the associated render resources and default transform
	bool init(int level) override;

	// Update enemy due to current
	// ms represents the number of milliseconds elapsed from the previous update() call
	void update(float ms, vec2 target_pos) override;

	// Renders the salmon
	// projection is the 2D orthographic projection matrix
	void draw(const mat3& projection)override;

	bool checkIfCanFire(clock_t currentClock);

    void setTextureLocs(int index);

	vec2 get_bounding_box()const;

	int powerup();

	bool poweredup;

	bool speedBoost;

	bool groupAtk;

	private:
	int powerupType;

	clock_t timePassed;

	float variation;

    TexturedVertex texVertices[4];

    std::vector<float> texture_locs;

    EnemyMoveState m_moveState;

    float m_animTime = 0.0f;

    int numTiles;

};