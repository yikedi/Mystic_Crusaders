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
#include "shop_data.hpp"

class Shop_screen : public Renderable
{
	
	static Texture shop_texture;


public:

	// Creates all the associated render resources and default transform
	bool init(vec2 screen);

	void destroy();

	void draw(const mat3& projection)override;

	void update_shop(bool shopping, int current_stock, int balance, int current_price, int item_num, vec2 screen);

	bool inside(vec2 h, vec2 w, vec2 pos);

	bool level_position(vec2 mouse_pos, vec2 screen);

	int item_position(vec2 mouse_pos, vec2 screen);

	vec2 set_scale(float w, float h, vec2 screen);

	vec2 stock_to_vec(int number);

	vec5 money_to_vec(int number);


private:


	Purchase purchase;
	Shop_frame shopf;
	Shop_item items;
	vec2 m_scale;
	vec2 m_position;

	TexturedVertex vertices[4];

	Shop_data stock1;
	Shop_data stock2;
	Shop_data price1;
	Shop_data price2;
	Shop_data price3;
	Shop_data price4;
	Shop_data price5;
	Shop_data balance1;
	Shop_data balance2;
	Shop_data balance3;
	Shop_data balance4;
	Shop_data balance5;
	int skillpoints;
	int used_skillpoints;
	int free_skillpoints;

};