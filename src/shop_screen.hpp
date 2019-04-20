#pragma once
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "common.hpp"
#include "purchase.hpp"
#include "shop_frame.hpp"
#include "shop_item.hpp"

class Shop_screen : public Renderable
{
	
	static Texture shop_texture;


public:

	// Creates all the associated render resources and default transform
	bool init(vec2 screen);

	// Releases all the associated resources
	void destroy();

	void draw(const mat3& projection)override;

	void update_shop(bool shopping, int current_stock, int afforable, int item_num, vec2 screen);

	bool inside(vec2 h, vec2 w, vec2 pos);

	bool level_position(vec2 mouse_pos, vec2 screen);

	int item_position(vec2 mouse_pos, vec2 screen);

	vec2 set_scale(float w, float h, vec2 screen);


private:


	Purchase purchase;
	Shop_frame shopf;
	Shop_item items;
	vec2 m_scale;
	vec2 m_position;

	TexturedVertex vertices[4];

	

	int skillpoints;
	int used_skillpoints;
	int free_skillpoints;

};