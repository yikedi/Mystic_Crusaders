#pragma once

#include "common.hpp"
#include "projectile.h"
#include "fireball.h"
#include "enemy_01.hpp"
#include "enemy_02.hpp"
#include "hero.hpp"

// actual used includes
#include <cmath>
#include <string>
#include <functional>


class Button : public Renderable
{

	static Texture button_texture;

public:
	
	typedef void(*ClickCallback)();
	using ClickCallbackSTD = std::function<void()>;
	Button(){}

	// Gets the top-left corner coordinates, width, height, and message for the button, and returns a button
	Button(int x, int y, int w, int h, std::string path, std::string type, ClickCallbackSTD onClick);

	// Destructor
	~Button();

	void destroy();

	// bool init(double x, double y, double w, double h, std::string text);

	bool init(double x, double y, double w, double h, std::string path, std::string type, ClickCallbackSTD onClick);

	void CheckClick(vec2 mouse_position);

	void set_color(vec3 color);

	void draw(const mat3 &projection);

	void set_position(vec2 position);


	/*
	// Releases all associated resources
	void destroy();

	// If mouse is hovering above the button, we will light up the button.
	void update(vec2 hp_mp, float zoom_factor);

	// Renders the salmon
	void draw(const mat3& projection)override;

	// Light up the button when is_within_range()
	void lightup();

	// Checks to see if mouse is within range. To be used in update of mouse and mouse_onclick
	bool is_within_range();
	*/

	ClickCallbackSTD onClick;
	float m_color[3];
	

private:
	double width;
	double height;
	double left_corner;
	double top_corner;
	std::string path;
	std::string type;
	float zoom_factor;
	
	vec2 m_position; // Window coordinates
	vec2 m_scale; // 1.f in each dimension. 1.f is as big as the associated texture
	float m_rotation; // in radians
	size_t m_num_indices; // passed to glDrawElements

	//add salmon speed
	vec2 m_direction;
	int m_light_up;
};
