#pragma once

#include "common.hpp"
#include "projectile.h"
#include "fireball.h"
#include "enemy_01.hpp"
#include "enemy_02.hpp"
#include "sprite_sheet.hpp"

class Enemy_01;
class Fish;

enum class HeroMoveState { STANDING, FRONTMOVING, BACKMOVING, LEFTMOVING, RIGHTMOVING, ATTACKING };

class Hero : public Renderable
{

	static SpriteSheet hero_texture;

public:
	// Creates all the associated render resources and default transform
	//hero attributes
	float max_hp;
	float max_mp;
	float hp;
	float mp;
	//std::vector<skill> skill_list;

    int numTiles = 1;

	float m_color[3];
	bool advanced;
	vec2 m_screen;
	bool init(vec2 screen);

	// Releases all associated resources
	void destroy();

	// Update salmon position based on direction
	// ms represents the number of milliseconds elapsed from the previous update() call
	void update(float ms);

	// Renders the salmon
	void draw(const mat3& projection)override;

	// Collision routines for enemys and fish
	bool collides_with(const Enemy_02& enemy);
	bool collides_with(Projectile &projectile);
	bool collides_with(const Fish& fish);

	// Returns the current salmon position
	vec2 get_position()const;

    // set Texture locations
    void setTextureLocs(int index);

	// Moves the salmon's position by the specified offset
	void move(vec2 off);

	// Set salmon rotation in radians
	void set_rotation(float radians);

	// True if the salmon is alive
	bool is_alive()const;

	// Kills the salmon, changing its alive state and triggering on death events
	void kill();

	// Called when the salmon collides with a fish, starts lighting up the salmon
	void light_up();

	void set_direction(vec2 velocity);

	vec2 get_direction();

    void set_moveState(HeroMoveState state);

    HeroMoveState get_moveState();

    bool mesh_collision(vec3 point, std::vector<vec3> &cur_vertices);
    void transform_current_vertex(std::vector<vec3> &cur_vertices);
	void set_color(vec3 color);

	//hero functions
	void take_damage(float damage);
	void change_hp(float d_hp);
	void change_mp(float d_mp);
	float get_hp();
	void apply_momentum(vec2 f);
	void set_speed(vec2 speed);
	bool shoot_projectiles(std::vector<Fireball> & hero_projectiles);

private:
    TexturedVertex texVertices[4];
	float m_light_up_countdown_ms; // Used to keep track for how long the salmon should be lit up
	bool m_is_alive; // True if the salmon is alive
	vec2 m_position; // Window coordinates
	vec2 m_scale; // 1.f in each dimension. 1.f is as big as the associated texture
	float m_rotation; // in radians
	size_t m_num_indices; // passed to glDrawElements

    // animation
    float m_animTime = 0.0f;
    HeroMoveState m_moveState = HeroMoveState::STANDING;

	//add salmon speed
	vec2 m_direction;
	int m_light_up;
};
