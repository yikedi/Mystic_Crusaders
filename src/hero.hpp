#pragma once

#include "common.hpp"
#include "projectile.h"
#include "fireball.h"
#include "enemy_01.hpp"
#include "enemy_02.hpp"
#include "vine.h"
#include "sprite_sheet.hpp"
#include "Ice_arrow_skill.h"
#include "Ice_arrow.h"
#include "Thunder.h"
#include "ThunderSkill.h"
#include "phoenix_skill.h"
#include "phoenix.h"
#include "time.h"

#define ICE_SKILL 0
#define THUNDER_SKILL 1
#define PHOENIX_SKILL 2

class Enemy_01;
class Fish;

enum class HeroMoveState { STANDING, FRONTMOVING, BACKMOVING, LEFTMOVING, RIGHTMOVING, DEAD };

class Hero : public Renderable
{

	static SpriteSheet hero_texture;

public:
	// Creates all the associated render resources and default transform
	//hero attributes
	float max_hp;
	float max_mp;
	float mp_recovery_rate;
	float hp;
	float mp;
	float fireball_damage;
	float movement_speed;
	float exp_multiplier;
	bool second_life;
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
	bool collides_with(Vine &vine);

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

	void set_position(vec2 position);

	vec2 get_position();

    HeroMoveState get_moveState();

	vec2 get_bounding_box();
    bool mesh_collision(vec3 point, std::vector<vec3> &cur_vertices);
    void transform_current_vertex(std::vector<vec3> &cur_vertices);
	void set_color(vec3 color);

	//hero functions
	void take_damage(float damage);
	void change_hp(float d_hp);
	void change_mp(float d_mp);
	float get_hp();
    float get_mp();
	void apply_momentum(vec2 f);

	bool shoot_projectiles(std::vector<Projectile*> & hero_projectiles);
	bool use_ice_arrow_skill(std::vector<Projectile*> & hero_projectiles);
    void levelup();
	bool use_thunder_skill(std::vector<Thunder*> & thunders, vec2 position);
	bool use_phoenix_skill(std::vector<phoenix*> & phoenix);
	bool use_skill(std::vector<Projectile*> & hero_projectiles, std::vector<Thunder*> & thunders, std::vector<phoenix*> &phoenix_list, vec2 position);
    void level_up(int select_skill, int select_upgrade);
	void set_active_skill(int active);
	int get_active_skill();
	void next_level();
	vec2 m_scale; // 1.f in each dimension. 1.f is as big as the associated texture
	int level;
	bool isInTransition;
	bool justFinishedTransition;


private:
    TexturedVertex texVertices[4];
    std::vector<float> texture_locs;
	float m_light_up_countdown_ms; // Used to keep track for how long the salmon should be lit up
	bool m_is_alive; // True if the salmon is alive
	vec2 m_position; // Window coordinates
	float m_rotation; // in radians
	size_t m_num_indices; // passed to glDrawElements

    // animation
    float m_animTime = 0.0f;
    HeroMoveState m_moveState = HeroMoveState::STANDING;

	//add salmon speed
	vec2 m_direction;
	int m_light_up;
    Ice_arrow_skill ice_arrow_skill;
	ThunderSkill thunder_skill;
	phoenix_skill phoenix_skill;
	int activeSkill;

	vec2 momentum;
    float deceleration;
    float momentum_factor;
	clock_t transition_time;
	float transition_duration;
	bool just_took_damage;
};
