#pragma once

#include "common.hpp"
#include "enemies.hpp"
#include "enemy_laser.h"
#include "projectile.h"

// Salmon enemy
class Enemy_01 : public Renderable, public Enemies
{
	// Shared between all enemys, no need to load one for each instance
	static Texture enemy_texture;

public:

	// Releases all the associated resources
	void destroy() override;

	// Creates all the associated render resources and default transform
	bool init(int level) override;

	// Update enemy due to current
	// ms represents the number of milliseconds elapsed from the previous update() call
	void update(float ms, vec2 target_pos) override;

	// Renders the salmon
	// projection is the 2D orthographic projection matrix
	void draw(const mat3& projection)override;


	bool collide_with(Projectile &projectile);

	bool shoot_projectiles(std::vector<EnemyLaser> & enemy_projectiles);

	bool needFireProjectile;

	void setLastFireProjectileTime(clock_t c);

	bool checkIfCanFire(clock_t currentClock);

	double attackCooldown;

	private:
	clock_t lastFireProjectileTime;
};