#pragma once

#include "common.hpp"
#include "projectile.h"
#include "fireball.h"
#include "enemy_01.hpp"
#include "enemy_02.hpp"
#include "hero.hpp"

// actual used includes
#include "screen_button.hpp"

#include <cmath>
#include "string.h"




// static Texture button_texture;

typedef void (*ClickCallback)();
using namespace std;
using ClickCallbackSTD = std::function<void()>;

// Gets the top-left corner coordinates, width, height, and message for the button, and returns a button
// Takes in: type is just for your own choosing (ie can later distinguish between "tutorial" and "quest speech")
//		path is for the path of our texture
//		onClick is for the function you'd like to put in; examples listed below for sample functions called in World.cpp
//			[this]() { this->initTrees(); } <- initializes trees on button click. Don't actually do this though
//			[this]() { this->m_hero.change_mp(80.f); } <- sets the mp of our hero to 80.f. 
//					The "this" provides context that we're using our class "World.cpp" here, and we can then call things like we do in the document.
//			[this]() { this->startGame(); } <- actually used in the game. Starts the game on button click
//			[&]() { display_tutorial = false; } <- sets a global variable in World.cpp.
void Button::makeButton(int x, int y, int w, int h, std::string path, std::string type, ClickCallbackSTD onClick) {// ClickCallback onClick) {
	if (path == "") {
		// Perhaps consider making just a regular white button in this case? Currently throws error
		fprintf(stderr, "CANNOT FIND BUTTON PATH AT POSITION X= %i , Y= %i \n", x, y);
	}
	else {
		init((double)x, (double)y, (double)w, (double)h, path, type, onClick);
	}
}

// Overloading with opacity
void Button::makeButton(int x, int y, int w, int h, float opacity1, std::string path, std::string type, ClickCallbackSTD onClick) {// ClickCallback onClick) {
	if (path == "") {
		fprintf(stderr, "CANNOT FIND BUTTON PATH AT POSITION X= %i , Y= %i \n", x, y);
	}
	else {
		is_transparency_enabled = true;
		opacity = opacity1;
		init((double)x, (double)y, (double)w, (double)h, path, type, onClick);
	}
}

// referenced help from https://codereview.stackexchange.com/questions/154623/custom-opengl-buttons
bool Button::init(double x, double y, double w, double h, std::string path1, std::string type1, ClickCallbackSTD onClick1) {

		left_corner = x;
		top_corner = y;
		width = w;
		height = h;
		path = path1;
		type = type1;
		onClick = onClick1;


	/* Generic loading code */
	// Load shared texture
	if (!button_texture.is_valid())
	{
		/*
		  CONVERSION FROM STATIC PATH TO DYNAMIC ONE
		  First convert the textures_path() to string, add it with std::string, 
		  and then convert it back to const char-star
		*/
		string s = string(textures_path()) + path;
		if (!button_texture.load_from_file(s.c_str()))
		{
			fprintf(stderr, "Failed to load button texture! pathname: %s", path);
			return false;
		}
	}

	// The position corresponds to the center of the texture
	float wr = width / (double)2; // UserInterface_texture.width * 0.5f;
	float hr = height / (double)2; // UserInterface_texture.height * 0.5f;

	TexturedVertex vertices[4];
	vertices[0].position = { -wr, +hr, -0.01f };
	vertices[0].texcoord = { 0.f, 1.f };
	vertices[1].position = { +wr, +hr, -0.01f };
	vertices[1].texcoord = { 1.f, 1.f, };
	vertices[2].position = { +wr, -hr, -0.01f };
	vertices[2].texcoord = { 1.f, 0.f };
	vertices[3].position = { -wr, -hr, -0.01f };
	vertices[3].texcoord = { 0.f, 0.f };

	// counterclockwise as it's the default opengl front winding direction
	uint16_t indices[] = { 0, 3, 1, 1, 3, 2 };

	// Clearing errors
	gl_flush_errors();

	// Vertex Buffer creation
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TexturedVertex) * 4, vertices, GL_STATIC_DRAW);

	// Index Buffer creation
	glGenBuffers(1, &mesh.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * 6, indices, GL_STATIC_DRAW);

	// Vertex Array (Container for Vertex + Index buffer)
	glGenVertexArrays(1, &mesh.vao);
	if (gl_has_errors()) {
		fprintf(stderr, "ERROR IN SCREENBUTTON INIT! GL HAS ERRORS");
		return false;
	}

	// Loading shaders
	if (!effect.load_from_file(shader_path("textured.vs.glsl"), shader_path("textured_button.fs.glsl")))
		return false;

	// Setting initial values
	zoom_factor = 1.f;
	m_scale.x = 1.f;
	m_scale.y = 1.f;
	set_position({ (float) x, (float) y });

	// Setting initial values, scale is negative to make it face the opposite way
	// 1.0 would be as big as the original texture
	set_color({ 1.0f,1.0f,1.0f });
	return true;
}



Button::~Button() {
}

void Button::destroy() {
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteBuffers(1, &mesh.vao);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);
}

void Button::check_click(vec2 mouse_position) {
	float mouse_x = mouse_position.x;
	float mouse_y = mouse_position.y;
	if (mouse_x >= left_corner && mouse_x <= left_corner + width &&
		mouse_y >= top_corner && mouse_y <= top_corner + height) {
		onClick();
	}
}

bool Button::mouse_inside_button(vec2 mouse_position) {
	float mouse_x = mouse_position.x;
	float mouse_y = mouse_position.y;
	if (mouse_x >= left_corner && mouse_x <= left_corner + width &&
		mouse_y >= top_corner && mouse_y <= top_corner + height) {
		return true;
	}
}

void Button::draw(const mat3 &projection)
{
	// if ((button_hoverable && mouse_hovering) || !button_hoverable) {
	transform_begin();
	transform_translate(m_position);
	transform_scale(m_scale);
	transform_end();

	// Setting shaders
	glUseProgram(effect.program);

	// Enabling alpha channel for textures
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	// Getting uniform locations for glUniform* calls
	GLint transform_uloc = glGetUniformLocation(effect.program, "transform");
	GLint color_uloc = glGetUniformLocation(effect.program, "fcolor");
	GLint projection_uloc = glGetUniformLocation(effect.program, "projection");

	// Setting vertices and indices
	glBindVertexArray(mesh.vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);

	// Input data location as in the vertex buffer
	GLint in_position_loc = glGetAttribLocation(effect.program, "in_position");
	GLint in_texcoord_loc = glGetAttribLocation(effect.program, "in_texcoord");
	glEnableVertexAttribArray(in_position_loc);
	glEnableVertexAttribArray(in_texcoord_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)0);
	glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)sizeof(vec3));

	// Enabling and binding texture to slot 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, button_texture.id);

	// Setting uniform values to the currently bound program
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform);

	float color[] = { 1.f, 1.f, 1.f, opacity };
	glUniform4fv(color_uloc, 1, color);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);

	// Drawing!
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
	// }
}


void Button::set_color(vec3 in_color)
{
	if (is_transparency_enabled) {
		float color[4] = { in_color.x,in_color.y,in_color.z, opacity };
		memcpy(m_color_transparent, color, sizeof(color));
	}
	else {
		float color[3] = { in_color.x,in_color.y,in_color.z };
		memcpy(m_color, color, sizeof(color));
	}

}

void Button::set_position(vec2 position)
{
	m_position = { position.x / zoom_factor + (float)width / (2.f * zoom_factor), position.y / zoom_factor + (float)height / (2.f * zoom_factor) };
}

void Button::set_hoverable(bool is_hoverable) {
	button_hoverable = is_hoverable;
}
