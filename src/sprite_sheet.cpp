// Header
#include "sprite_sheet.hpp"

#include "../ext/stb_image/stb_image.h"

// stlib
#include <vector>
#include <string>
#include <algorithm>

bool SpriteSheet::load_from_file(const char * path)
{
    if (path == nullptr)
        return false;

    stbi_uc* data = stbi_load(path, &width, &height, NULL, 4);
    if (data == NULL)
        return false;

    gl_flush_errors();
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    stbi_image_free(data);
    return !gl_has_errors();
}

bool SpriteSheet::updateTexture(const char* path, int index)
{
    if (path == nullptr)
        return false;

    stbi_uc* data = stbi_load(path, &width, &height, NULL, 4);
    if (data == NULL)
        return false;

    int tileWidth = width / totalTiles;
    int xOffset = - width / 2 + tileWidth / 2 + tileWidth * index;

    glBindTexture(GL_TEXTURE_2D, id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, xOffset, 0, tileWidth, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    stbi_image_free(data);
    return !gl_has_errors();


}
