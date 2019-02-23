#pragma once

#include "common.hpp"

struct SpriteSheet : public Texture {
    int totalTiles;
    int currIndex;
    bool load_from_file(const char* path);
    bool updateTexture(const char* path);
};