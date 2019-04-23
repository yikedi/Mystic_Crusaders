#pragma once

#include "common.hpp"

// actual used includes
#include <cmath>
#include <string>
#include <functional>


class Button : public Renderable
{

	Texture button_texture;

public:
	
	typedef void(*ClickCallback)();
	using ClickCallbackSTD = std::function<void()>;
	Button(){}

	/* Gets the top-left corner coordinates, width, height, and message for the button, and returns a button
		Needs std::bind to work! USE SYNTAX LIKE THIS: std::bind(&functionClass::functionName, this, (optional variable parameters to pass in) )
	*/
	void makeButton(int x, int y, int w, int h, std::string path, std::string type, ClickCallbackSTD onClick);

	// Make a button with transparency involved
	void makeButton(int x, int y, int w, int h, float opacity1, std::string path, std::string type, ClickCallbackSTD onClick);

	// Destructor
	~Button();

	void destroy();

	// bool init(double x, double y, double w, double h, std::string text);

	bool init(double x, double y, double w, double h, std::string path, std::string type, ClickCallbackSTD onClick);

	void check_click(vec2 mouse_position);

	bool mouse_inside_button(vec2 mouse_position);

	void set_color(vec3 color);

	void draw(const mat3 &projection);

	void set_position(vec2 position);
	
	void set_hoverable(bool is_hoverable);

	ClickCallbackSTD onClick;
	float m_color[3];
	float m_color_transparent[4];
	bool mouse_hovering = false;
	

private:
	double width;
	double height;
	double left_corner;
	double top_corner;
	std::string path;
	std::string type;
	float zoom_factor;
	float opacity = 1.f; // 1 is fully opaque, and 0 is transparent
	bool is_transparency_enabled = false;
	bool button_hoverable = false;
	
	vec2 m_position; // Window coordinates
	vec2 m_scale; // 1.f in each dimension. 1.f is as big as the associated texture
	float m_rotation; // in radians
	size_t m_num_indices; // passed to glDrawElements

	//add salmon speed
	vec2 m_direction;
	int m_light_up;
};
