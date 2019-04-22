#include "skilltree.hpp"

#include <iostream>

#include <string.h>
#include <cassert>
#include <sstream>
#include <cmath>

#include <gl3w.h>
Texture Skilltree::screen_texture;


bool Skilltree::init(vec2 screen, int element)
{
	switch (element) {
	case 1:
		screen_texture.load_from_file(element1());
		front_element = "ice";
		upper_element = "fire";
		lower_element = "thunder";
		break;
	case 2:
		screen_texture.load_from_file(element2());
		front_element = "thunder";
		upper_element = "ice";
		lower_element = "fire";
		break;
	case 3:
		screen_texture.load_from_file(element3());
		front_element = "fire";
		upper_element = "thunder";
		lower_element = "ice";
		break;
	}
	float w = screen_texture.width;
	float h = screen_texture.height;
	float wr = w * 0.5f;
	float hr = h * 0.5f;

	TexturedVertex vertices[4];
	vertices[0].position = { -wr, +hr, 0.f };
	vertices[0].texcoord = { 0.f, 1.f };
	vertices[1].position = { +wr, +hr, 0.f };
	vertices[1].texcoord = { 1.f, 1.f };
	vertices[2].position = { +wr, -hr, 0.f };
	vertices[2].texcoord = { 1.f, 0.f };
	vertices[3].position = { -wr, -hr, 0.f };
	vertices[3].texcoord = { 0.f, 0.f };

	// counterclockwise as it's the default opengl front winding direction
	uint16_t indices[] = { 0, 3, 1, 1, 3, 2 };

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
	if (gl_has_errors())
		return false;

	// Loading shaders
	if (!effect.load_from_file(shader_path("textured.vs.glsl"), shader_path("textured.fs.glsl")))
		return false;

	m_scale = {1.f, 1.f};
	m_position.x = screen.x/2;
	m_position.y = screen.y/2;
	if (front_element == "ice") {
		ices1.init(screen, 1);
		ices2.init(screen, 2);
		ices3.init(screen, 3);
	}
	else if (front_element == "thunder") {
		thunder1.init(screen, 1);
		thunder2.init(screen, 2);
		thunder3.init(screen, 3);
	}
	else if (front_element == "fire") {
		fire1.init(screen, 1);
		fire2.init(screen, 2);
		fire3.init(screen, 3);
	}
	skillup.init(screen);
	description.init(screen);
	return true;
}

void Skilltree::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);
    glDeleteBuffers(1, &mesh.ibo);

    effect.release();

	ices1.destroy();
	ices2.destroy();
	ices3.destroy();
	thunder1.destroy();
	thunder2.destroy();
	thunder3.destroy();
	fire1.destroy();
	fire2.destroy();
	fire3.destroy();
	skillup.destroy();
	description.destroy();
}

void Skilltree::draw(const mat3 & projection)
{
		gl_flush_errors();

		transform_begin();
		transform_translate(m_position);
		transform_scale(m_scale);
		transform_end();

		glUseProgram(effect.program);

		// Enabling alpha channel for textures
		glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
		glBindTexture(GL_TEXTURE_2D, screen_texture.id);

		// Setting uniform values to the currently bound program
		glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform);
		float color[] = { 1.f, 1.f, 1.f };
		glUniform3fv(color_uloc, 1, color);
		glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);

		// Drawing!
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
		if (front_element == "ice") {
			ices1.draw(projection);
			ices2.draw(projection);
			ices3.draw(projection);
		}
		else if (front_element == "thunder") {
			thunder1.draw(projection);
			thunder2.draw(projection);
			thunder3.draw(projection);
		}
		else if (front_element == "fire") {
			fire1.draw(projection);
			fire2.draw(projection);
			fire3.draw(projection);
		}
		skillup.draw(projection);
		description.draw(projection);
}

void Skilltree::update_skill(bool paused, int total, int used, vec3 ice_num, vec3 thunder_num, vec3 fire_num, int skill_num, vec2 screen)
{
	if (paused) {
		if (front_element == "ice") {
			ices1.update_ice(paused, ice_num.x);
			ices2.update_ice(paused, ice_num.y);
			ices3.update_ice(paused, ice_num.z);
		}
		else if (front_element == "thunder") {
			thunder1.update_ice(paused, thunder_num.x);
			thunder2.update_ice(paused, thunder_num.y);
			thunder3.update_ice(paused, thunder_num.z);
		}
		else if (front_element == "fire") {
			fire1.update_ice(paused, fire_num.x);
			fire2.update_ice(paused, fire_num.y);
			fire3.update_ice(paused, fire_num.z);
		}
		skillup.update_leveltex(paused, total - used, skill_num);
		description.update_description(paused, skill_num, screen);
	}
	else {
		ices1.blue_up();
		ices2.blue_up();
		ices3.blue_up();
		thunder1.blue_up();
		thunder2.blue_up();
		thunder3.blue_up();
		fire1.blue_up();
		fire2.blue_up();
		fire3.blue_up();
	}
}

std::string Skilltree::get_element()
{
	return front_element;
}

bool Skilltree::inside(vec2 h, vec2 w,vec2 pos)
{
	float x = pos.x;
	float y = pos.y;
	if (x<w.x && y<h.x && x>w.y && y>h.y) {
		return true;
	}
	else {
		return false;
	}
}

bool Skilltree::level_position(vec2 mouse_pos)
{
	float lxpos = skillup.get_position().x;
	float lypos = skillup.get_position().y;
	vec2 height = { lypos + 40.f,lypos-40.f };
	vec2 width = { lxpos + 192.5f, lxpos -192.5f};
	if (inside(height,width,mouse_pos)) {
		ices1.blue_up();
		ices2.blue_up();
		ices3.blue_up();
		thunder1.blue_up();
		thunder2.blue_up();
		thunder3.blue_up();
		fire1.blue_up();
		fire2.blue_up();
		fire3.blue_up();
		return true;
	}
	else {
		ices1.blue_up();
		ices2.blue_up();
		ices3.blue_up();
		thunder1.blue_up();
		thunder2.blue_up();
		thunder3.blue_up();
		fire1.blue_up();
		fire2.blue_up();
		fire3.blue_up();
		return false;
	}

}

int Skilltree::icon_position(vec2 mouse_pos, std::string element)
{
	float lxpos1 = ices1.get_position().x;
	float lypos1 = ices1.get_position().y;

	float lxpos2 = ices2.get_position().x;
	float lypos2 = ices2.get_position().y;

	float lxpos3 = ices3.get_position().x;
	float lypos3 = ices3.get_position().y;

	vec2 height1 = { lypos1 + 30.f,lypos1 - 30.f };
	vec2 width1  = { lxpos1 + 30.f,lxpos1 - 30.f };

	vec2 height2 = { lypos2 + 30.f,lypos2 - 30.f };
	vec2 width2  = { lxpos2 + 30.f,lxpos2 - 30.f };

	vec2 height3 = { lypos3 + 30.f,lypos3 - 30.f };
	vec2 width3	 = { lxpos3 + 30.f,lxpos3 - 30.f };
	if (element == "ice") {
		if (inside(height1, width1, mouse_pos)) {
			ices1.light_up();
			ices2.blue_up();
			ices3.blue_up();
			return 1;
		}
		else if (inside(height2, width2, mouse_pos)) {
			ices1.blue_up();
			ices2.light_up();
			ices3.blue_up();
			return 2;
		}
		else if (inside(height3, width3, mouse_pos)) {
			ices1.blue_up();
			ices2.blue_up();
			ices3.light_up();
			return 3;
		}
		else {
			ices1.blue_up();
			ices2.blue_up();
			ices3.blue_up();
			return 0;
		}
	}
	else if (element == "thunder") {
		if (inside(height1, width1, mouse_pos)) {
			thunder1.light_up();
			thunder2.blue_up();
			thunder3.blue_up();
			return 1;
		}
		else if (inside(height2, width2, mouse_pos)) {
			thunder1.blue_up();
			thunder2.light_up();
			thunder3.blue_up();
			return 2;
		}
		else if (inside(height3, width3, mouse_pos)) {
			thunder1.blue_up();
			thunder2.blue_up();
			thunder3.light_up();
			return 3;
		}
		else {
			thunder1.blue_up();
			thunder2.blue_up();
			thunder3.blue_up();
			return 0;
		}
	}
	else if (element == "fire") {
		if (inside(height1, width1, mouse_pos)) {
			fire1.light_up();
			fire2.blue_up();
			fire3.blue_up();
			return 1;
		}
		else if (inside(height2, width2, mouse_pos)) {
			fire1.blue_up();
			fire2.light_up();
			fire3.blue_up();
			return 2;
		}
		else if (inside(height3, width3, mouse_pos)) {
			fire1.blue_up();
			fire2.blue_up();
			fire3.light_up();
			return 3;
		}
		else {
			fire1.blue_up();
			fire2.blue_up();
			fire3.blue_up();
			return 0;
		}
	}
}

std::string Skilltree::element_position(vec2 mouse_pos)
{
	float lxpos1 = 400.f;
	float lypos1 = 270.f;

	float lxpos2 = 400.f;
	float lypos2 = 540.f;

	float lxpos3 = 520.f;
	float lypos3 = 405.f;

	vec2 height1 = { lypos1 + 60.f,lypos1 - 60.f };
	vec2 width1 = { lxpos1 + 60.f,lxpos1 - 60.f };

	vec2 height2 = { lypos2 + 60.f,lypos2 - 60.f };
	vec2 width2 = { lxpos2 + 60.f,lxpos2 - 60.f };

	vec2 height3 = { lypos3 + 60.f,lypos3 - 60.f };
	vec2 width3 = { lxpos3 + 60.f,lxpos3 - 60.f };

	bool click_upper = inside(height1, width1, mouse_pos);
	bool click_lower = inside(height2, width2, mouse_pos);
	bool click_front = inside(height3, width3, mouse_pos);

	if (click_front) {
		return front_element;
	}
	else if (click_lower) {
		if (front_element == "ice") {
			front_element = "fire";
		}
		else if (front_element == "fire") {
			front_element = "thunder";
		}
		else if (front_element == "thunder") {
			front_element = "ice";
		}
	}
	else if (click_upper) {
		if (front_element == "ice") {
			front_element = "thunder";
		}
		else if (front_element == "fire") {
			front_element = "ice";
		}
		else if (front_element == "thunder") {
			front_element = "fire";
		}
	}
	return front_element;
}