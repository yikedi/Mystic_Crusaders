//
// Created by douglas on 19/2/21.
//

#ifndef INC_436D_Ice_arrow_H
#define INC_436D_Ice_arrow_H


#include "projectile.h"

class Ice_arrow: public Projectile
{
    static Texture texture;

public :

    Ice_arrow(float radius, float projectileSpeed = 400.f, float damage = 40.f)
    {
        init(radius,projectileSpeed, damage);
    }
    bool init(float radius, float projectileSpeed, float damage);

    // Renders the fish
    // projection is the 2D orthographic projection matrix
    void draw(const mat3& projection);

    // Returns the fish' bounding box for collision detection, called by collides_with()
    vec2 get_bounding_box()const;


};


#endif //INC_436D_Ice_arrow_H
