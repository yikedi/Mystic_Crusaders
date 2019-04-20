#include "Shop.h"
#include "project_path.hpp"

using namespace std;
#define JSON_FILE_PATH PROJECT_SOURCE_DIR "src/json/shop.json"

Shop::Shop()
{
}

bool Shop::init()
{
	ifstream ifs;
	
	ifs.open(JSON_FILE_PATH);
	Json::Reader reader;
	if (!reader.parse(ifs, shop_info, false))
		return false;

	buy_item("mp_recovery");
	ifs.close();

	return true;

}

void Shop::change_stock(string item_name,int value)
{
	shop_info["items"][item_name]["stock"] = value;
}

void Shop::change_price(string item_name,int value)
{
	shop_info["items"][item_name]["price"] = value;
}

void Shop::set_balance(int value)
{
	shop_info["balance"] = value;
}

int Shop::get_balance()
{
	return shop_info["balance"].asInt();
}
bool Shop::buy_item(string item_name)
{
	int stock = shop_info["items"][item_name]["stock"].asInt();
	int price = shop_info["items"][item_name]["price"].asInt();
	int balance = get_balance();
	if (balance >= price && stock > 0)
	{
		set_balance(balance - price);
		change_stock(item_name, stock - 1);
		change_price(item_name, int(price * 1.3));
		return true;
	}
	return false;
}

void Shop::save()
{	
	Json::StyledWriter writer;
	string output = writer.write(shop_info);
	ofstream ofs;
	ofs.open(JSON_FILE_PATH);
	ofs << output;
	ofs.close();
}

void Shop::update_hero(Hero& hero)
{
	Json::Value info_for_hero;
	int purchased = get_purchased("max_hp");
	hero.max_hp = 100 + purchased * get_interest_value("max_hp");
	purchased = get_purchased("mp_recovery");
	hero.mp_recovery_rate = 0.05 * (1 + purchased * get_interest_value("mp_recovery"));
	purchased = get_purchased("fireball_damage");
	hero.fireball_damage = 20.0f * (1.0 + float(purchased) * get_interest_value("fireball_damage"));
	purchased = get_purchased("movement_speed");
	hero.movement_speed = 200.f * (1.0 + float(purchased) * get_interest_value("movement_speed"));
	purchased = get_purchased("exp_increase");
	hero.exp_multiplier = 1.0 + float(purchased) * get_interest_value("exp_increase");
	purchased = get_purchased("second_life");
	bool second_life(purchased > 0);
	hero.second_life = second_life;
}

int Shop::get_purchased(string item_name)
{
	return shop_info["items"][item_name]["max_stock"].asInt() - shop_info["items"][item_name]["stock"].asInt();
}

float Shop::get_interest_value(string item_name)
{
	return shop_info["items"][item_name]["interest_value"].asFloat();
}

int Shop::get_price(string item_name)
{
	return shop_info["items"][item_name]["price"].asInt();
}

int Shop::get_stock(string item_name)
{
	return shop_info["items"][item_name]["stock"].asInt();
}

int Shop::get_maxstock(string item_name)
{
	return shop_info["items"][item_name]["max_stock"].asInt();
}

Shop::~Shop()
{
}
