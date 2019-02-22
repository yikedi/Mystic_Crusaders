//
// Created by douglas on 19/2/22.
//

#include "Ice_arrow_skill.h"
#include "Ice_arrow.h"

void Ice_arrow_skill::init()
{
    mp_cost = 2;
    damage = 20.f;
    level = 0;
    num_arrows = 3;
    shoot_range_angle = (float) M_PI * 15 /180;
}

float Ice_arrow_skill::get_damage() {return damage;}

void Ice_arrow_skill::set_damage(float m_damage) {damage = m_damage;}

void Ice_arrow_skill::set_mp_cost(float cost) {mp_cost = cost;}

void Ice_arrow_skill::set_range_angle(float radius) {shoot_range_angle = radius;}

float Ice_arrow_skill::get_mpcost(){return mp_cost;}

float Ice_arrow_skill::shoot_ice_arrow(std::vector<Projectile *> &hero_projectiles, float radius, vec2 position)
{
    float d_angle = shoot_range_angle/(num_arrows-1);
    float start_angle = radius - shoot_range_angle;
    for (int i= 0; i<num_arrows; i++)
    {
        float current_angle = start_angle + d_angle * i;
        Ice_arrow* ice_arrow = new Ice_arrow(current_angle, 400.f, damage);
        ice_arrow->set_position(position);
        hero_projectiles.emplace_back(ice_arrow);
    }
    return mp_cost;

}

void Ice_arrow_skill::level_up()
{
    if (level < 4)
    {
        level++;
        damage += 20;
        num_arrows += 2;
        shoot_range_angle += (float) M_PI * 15 /180;
        mp_cost += 3;
    }
}