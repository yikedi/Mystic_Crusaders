#include "shop_screen.hpp"

#include <iostream>

#include <string.h>
#include <cassert>
#include <sstream>
#include <cmath>

#include <gl3w.h>
Texture Shop_screen::shop_texture;

bool Shop_screen::init(vec2 screen)
{
	if (!shop_texture.is_valid())
	{
		if (!shop_texture.load_from_file(textures_path("shop.png")))
		{
			fprintf(stderr, "Failed to load shop texture!");
			return false;
		}
	}

	float w = shop_texture.width;
	float h = shop_texture.height;
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

	m_scale = set_scale(w, h, screen);
	m_position.x = screen.x/2;
	m_position.y = screen.y/2;
	shopf.init(screen);
	items.init(screen);
	purchase.init(screen);
	return true;
}

void Shop_screen::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);
    glDeleteBuffers(1, &mesh.ibo);

    glDeleteShader(effect.vertex);
    glDeleteShader(effect.fragment);
    glDeleteShader(effect.program);

	purchase.destroy();
	shopf.destroy();
	items.destroy();
}

void Shop_screen::draw(const mat3 & projection)
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
		glBindTexture(GL_TEXTURE_2D, shop_texture.id);

		// Setting uniform values to the currently bound program
		glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform);
		float color[] = { 1.f, 1.f, 1.f };
		glUniform3fv(color_uloc, 1, color);
		glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);

		// Drawing!
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

		purchase.draw(projection);
		shopf.draw(projection);
		items.draw(projection);
}

void Shop_screen::update_shop(bool shopping, int current_stock, int afforable, int item_num, vec2 screen)
{
	if (shopping) {
		shopf.update_sframe(shopping, item_num, screen);
		items.update_item(shopping, item_num);
		purchase.update_purchase(shopping, current_stock, afforable, item_num);
	}
}

bool Shop_screen::inside(vec2 h, vec2 w,vec2 pos)
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

bool Shop_screen::level_position(vec2 mouse_pos, vec2 screen)
{
	float lxpos = purchase.get_position().x;
	float lypos = purchase.get_position().y;
	vec2 height = { lypos + 40.f,lypos - 40.f };
	vec2 width = { lxpos + 192.5f, lxpos - 192.5f };
	if (inside(height, width, mouse_pos)) {
		return true;
	}
	else {
		return false;
	}

}

int Shop_screen::item_position(vec2 mouse_pos, vec2 screen)
{
	float x = screen.x;
	float y = screen.y;
	vec2 height1 = { y / 4 + 60.f, y / 4 - 60.f };
	vec2 width1  = { 0.375*x + 60.f, 0.375*x - 60.f };

	vec2 height2= { y / 4 + 60.f, y / 4 - 60.f };
	vec2 width2 = { x / 2 + 60.f, x / 2 - 60.f };

	vec2 height3 = { y / 4 + 60.f, y / 4 - 60.f };
	vec2 width3 = { 0.68*x + 60.f, 0.68*x - 60.f };

	vec2 height4 = { y / 2 + 60.f, y / 2 - 60.f };
	vec2 width4  = { 0.375*x + 60.f, 0.375*x - 60.f };

	vec2 height5 = { y / 2 + 60.f, y / 2 - 60.f };
	vec2 width5  = { x / 2 + 60.f, x / 2 - 60.f };

	vec2 height6 = { y / 2 + 60.f, y / 2 - 60.f };
	vec2 width6  = { 0.68*x + 60.f, 0.68*x - 60.f };

	if (inside(height1, width1, mouse_pos)) {
		return 1;
	}
	else if (inside(height2, width2, mouse_pos)) {
		return 2;
	}
	else if (inside(height3, width3, mouse_pos)) {
		return 3;
	}
	else if (inside(height4, width4, mouse_pos)) {
		return 4;
	}
	else if (inside(height5, width5, mouse_pos)) {
		return 5;
	}
	else if (inside(height6, width6, mouse_pos)) {
		return 6;
	}
	else {
		return 0;
	}
}

vec2 Shop_screen::set_scale(float w, float h, vec2 screen)
{
	float xscale = screen.x / w;
	float yscale = screen.y / h;
	return { xscale, yscale };
}