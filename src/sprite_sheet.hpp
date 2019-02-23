#pragma once

#include "common.hpp"

struct SpriteSheet : public Texture {
    int totalTiles;
    bool load_from_file(const char* path, int index);
    bool updateTexture(const char* path, int index);
};