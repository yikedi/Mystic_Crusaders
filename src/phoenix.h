#pragma once
#include "common.hpp"
#include "sprite_sheet.hpp"
#include "projectile.h"
#include "particles.h"
#include "enemies.hpp"
#include "enemy_01.hpp"
#include "enemy_02.hpp"
#include "enemy_03.hpp"

class phoenix : public Renderable
{
	static SpriteSheet texture;
public:
	phoenix() {};
	phoenix(float hp, float damage,vec2 position,vec2 scale);
	~phoenix();

	bool init(float hp,float damage,vec2 position,vec2 scale);

	void set_position(vec2 position);

	vec2 get_position();

	vec2 find_target(std::vector<Enemy_01> &m_enemys_01, std::vector<Enemy_02> &m_enemys_02, std::vector<Enemy_03> &m_enemys_03);

	void attack(std::vector<Enemy_01> &m_enemys_01, std::vector<Enemy_02> &m_enemys_02, std::vector<Enemy_03> &m_enemys_03, std::vector<Projectile*> & hero_projectiles);

	void update(float ms);

	void draw(const mat3 &projection);

	void destroy();
	
	void shoot_projectiles(vec2 target_position, std::vector<Projectile*> & hero_projectiles);

	void emit_particles();

	void change_hp(float d_hp);

	vec2 get_bounding_box();

	void setTextureLocs(int index);

protected:
	float m_hp;
	float max_hp;
	float particle_damage;
	float projectile_damage;
	vec2 m_position;
	vec2 m_scale;
	float m_rotation;
	float speed;
	float elapsedTime;
	float animation_time;
	TexturedVertex texVertices[4];
	std::vector<float> texture_locs;
	bool first_time;
	std::vector<particles> m_particles;
};

