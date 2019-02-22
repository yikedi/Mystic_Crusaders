//
// Created by douglas on 19/2/22.
//

#ifndef INC_436D_ICE_ARROW_SKILL_H
#define INC_436D_ICE_ARROW_SKILL_H

#include "projectile.h"
#include <cmath>

class Ice_arrow_skill {
public:
    Ice_arrow_skill()
    {
        mp_cost = 2;
        damage = 40.f;
        level = 0;
        num_arrows = 3;
        shoot_range_angle = (float) M_PI * 15 /180;
    }

    void init();

    void level_up();

    void set_mp_cost(float cost);

    void set_damage(float damage);

    void set_range_angle(float radius);

    float get_damage();

    float get_mpcost();

    float shoot_ice_arrow(std::vector<Projectile*> &hero_projectiles, float radius, vec2 position);

protected:
    float mp_cost;
    float damage;
    int level;
    int num_arrows;
    float shoot_range_angle;
};


#endif //INC_436D_ICE_ARROW_SKILL_H
