//
// Created by douglas on 19/2/22.
//

#ifndef INC_436D_ICE_ARROW_SKILL_H
#define INC_436D_ICE_ARROW_SKILL_H
#define _USE_MATH_DEFINES

#include "projectile.h"
#include <cmath>
#include <math.h>
#include "Skill.h"

class Ice_arrow_skill: public Skill{
public:
    Ice_arrow_skill()
    {
		init();
    }

    void init();

	bool level_up(int select);

    void set_range_angle(float radius);

    float shoot_ice_arrow(std::vector<Projectile*> &hero_projectiles, float radius, vec2 position);

protected:
    int num_arrows;
    float shoot_range_angle;
};


#endif //INC_436D_ICE_ARROW_SKILL_H
