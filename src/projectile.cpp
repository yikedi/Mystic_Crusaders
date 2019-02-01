//
// Created by douglas on 19/1/27.
//

#include "projectile.h"
#include <cmath>

vec2 Projectile::get_position()
{
    return m_position;
}

void Projectile::set_position(vec2 position)
{
    m_position = position;
}

void Projectile::update(float ms)
{
    // Move fish along -X based on how much time has passed, this is to (partially) avoid
    // having entities move at different speed based on the machine.
    // const float TURTLE_SPEED = 200.f;
    float stepx = velocity.x * (ms / 1000);
    float stepy = velocity.y * (ms / 1000);
    m_position.x += stepx;
    m_position.y += stepy;
}

