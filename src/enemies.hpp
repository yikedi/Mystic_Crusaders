#pragma once
class Thunder;
#include "common.hpp"
#include "sprite_sheet.hpp"
#include "time.h"
#include "projectile.h"

enum class EnemyMoveState { STANDING, FRONTMOVING, BACKMOVING, LEFTMOVING, RIGHTMOVING, ATTACKING };

// general enemies
class Enemies : public Renderable
{
	// Shared between all enemys, no need to load one for each instance
	static Texture enemy_texture;

public:

    virtual bool init(int level) = 0;

    virtual void update(float ms, vec2 target_pos) = 0;

    virtual void destroy() = 0;

	// Returns the current enemy position
	vec2 get_position()const;

	// Sets the new enemy position
	void set_position(vec2 position);

    // 0 is left, 1 is right
    void set_facing(int rotation_value);

	// Returns the enemy' bounding box for collision detection, called by collides_with()
	vec2 get_bounding_box()const;

    void take_damage(float damage, vec2 direction);

	void take_damage(float damage);

    bool is_alive()const;

    void set_speed(float speed);

	float get_speed();

    void apply_momentum(vec2 momentum);

    void set_rotation(float rotation_value);

    void setRandMovementTime(clock_t c);

	bool checkIfCanChangeDirectionOfMove(clock_t currentClock);

	vec2 get_momentum();

	vec2 get_scale();
	virtual bool collide_with(Projectile &projectile);

	virtual bool collide_with(Thunder & thunder);

	void set_stunded(bool hit);

public:
	vec2 m_position; // Window coordinates
	vec2 m_scale; // 1.f in each dimension. 1.f is as big as the associated texture
    float m_speed; // enemy speed
	int m_face_left_or_right; // 0 is left, 1 is right
    float m_rotation; // facing hero's angle, use for shoot projectile and attack
    float hp;
    bool m_is_alive; // True if the enemy is alive
    float randMovementCooldown;
    clock_t randMovementTime;
    float enemyRandMoveAngle;
    vec2 momentum;
    float deceleration;
    float momentum_factor;
    int m_level;
	bool stunned;

};