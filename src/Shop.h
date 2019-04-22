#pragma once
#include<iostream>
#include<fstream>
#include<string>
#include "json/json.h"
#include "hero.hpp"
using namespace std;

class Shop
{
public:
	Shop();
	bool init();
	void change_stock(string item_name, int value);
	void change_price(string item_name, int value);
	void set_balance(int value);
	int get_balance();
	int get_purchased(string item_name);
	float get_interest_value(string item_name);
	int get_price(string item_name);
	void save();
	bool buy_item(string key);
	void update_hero(Hero& hero);
	int get_stock(string item_name);
	int get_maxstock(string item_name);
	~Shop();

	Json::Value shop_info;
};

