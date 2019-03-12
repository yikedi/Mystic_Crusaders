//
// Created by douglas on 19/1/27.
//

#ifndef INC_436D_PROJECTILE_H
#define INC_436D_PROJECTILE_H

#include "common.hpp"
#include <cmath>


class Projectile: public Renderable
{
    static Texture texture;

public:
    // Creates all the associated render resources and default transform

    virtual bool init(float radius, float projectileSpeed, float damage) = 0;

    // Releases all the associated resources
    virtual void destroy();

    // Update enemy due to current
    // ms represents the number of milliseconds elapsed from the previous update() call
    virtual void update(float ms);

    // Renders the salmon
    // projection is the 2D orthographic projection matrix
    virtual void draw(const mat3& projection) = 0;

    // Returns the current enemy position
    virtual vec2 get_position();

    virtual void set_position(vec2 position);

    // Returns the enemy' bounding box for collision detection, called by collides_with()
    virtual vec2 get_bounding_box() const = 0;

    vec2 get_velocity() const;

    float get_damage() const;

	vec2 get_scale() { return m_scale; }

protected:
    vec2 m_position; // Window coordinates
    vec2 m_scale; // 1.f in each dimension. 1.f is as big as the associated texture
    float m_rotation; // in radians
    vec2 velocity;
    float initial_speed;
    float damage;

};


#endif //INC_436D_PROJECTILE_H
