#include "skilltree.hpp"

#include <iostream>

#include <string.h>
#include <cassert>
#include <sstream>
#include <cmath>

#include <gl3w.h>
Texture Skilltree::screen_texture;

bool Skilltree::init(vec2 screen)
{
	screen_texture.load_from_file(textures_path("skill_tree.png"));
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

	//m_scale = set_scale(w, h, screen);
	m_scale = set_scale(1.1*w, 1.1*h, screen);
	m_position.x = screen.x/2;
	m_position.y = screen.y/2;
	ices1.init(screen, 1);
	ices2.init(screen, 2);
	ices3.init(screen, 3);
	skillup.init(screen);

	return true;
}

void Skilltree::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteBuffers(1, &mesh.vao);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);
	ices1.destroy();
	ices2.destroy();
	ices3.destroy();
	skillup.destroy();
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

		ices1.draw(projection);
		ices2.draw(projection);
		ices3.draw(projection);
		skillup.draw(projection);
}

void Skilltree::update_skill(bool paused, int total, int used, vec3 ice_num)
{
	//level0: only basic skill
	//level1: one element is chose by the player, degree will all be 0
	//level2: degree can be 1 2 3
	if (paused) {
		ices1.update_ice(paused, ice_num.x);
		ices2.update_ice(paused, ice_num.y);
		ices3.update_ice(paused, ice_num.z);
		skillup.update_leveltex(paused, total - used);
	}
}

std::string Skilltree::get_element()
{
	return element;
}

vec2 Skilltree::set_scale(float w, float h, vec2 screen)
{
	// temp code, will change after get window is possible
	float xscale = screen.x / w;
	float yscale = screen.y / h;
	//return{ 1.f,1.f };
	return { xscale, yscale };
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

bool Skilltree::level_position(vec2 mouse_pos, int used)
{
	float lxpos = skillup.get_position().x;
	float lypos = skillup.get_position().y;
	vec2 height = { lypos + 40.f,lypos-40.f };
	vec2 width = { lxpos + 192.5f, lxpos -192.5f};
	if (inside(height,width,mouse_pos)) {
		return true;
	}
	return false; 
}

bool Skilltree::ice_position(vec2 mouse_pos, int used)
{
	return false;
}
