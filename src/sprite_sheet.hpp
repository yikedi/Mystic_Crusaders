#pragma once

#include "common.hpp"

struct SpriteSheet : public Texture {
    int totalTiles;
    int subWidth;
    int subHeight;
};