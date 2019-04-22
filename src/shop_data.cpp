#include "shop_data.hpp"

#include <iostream>

#include <string.h>
#include <cassert>
#include <sstream>
#include <cmath>

#include <gl3w.h>

bool Shop_data::init(vec2 screen, int which)
{
	number_texture.load_from_file(textures_path("number_yellow.png"));
	switch (which) {
	case 1:							// stock
		m_position.x = (float)0.18*screen.x;
		m_position.y = (float)0.51*screen.y;
		break;
	case 2:							// stock
		m_position.x = (float)0.20*screen.x;
		m_position.y = (float)0.51*screen.y;
		break;
	case 3:							// p wan
		m_position.x = (float)0.15*screen.x;
		m_position.y = (float)0.466*screen.y;
		break;
	case 4:							// p qian
		m_position.x = (float)0.17*screen.x;
		m_position.y = (float)0.466*screen.y;
		break;
	case 5:							// p bai
		m_position.x = (float)0.19*screen.x;
		m_position.y = (float)0.466*screen.y;
		break;
	case 6:							// p shi
		m_position.x = (float)0.21*screen.x;
		m_position.y = (float)0.466*screen.y;
		break;
	case 7:							//p ge
		m_position.x = (float)0.23*screen.x;
		m_position.y = (float)0.466*screen.y;
		break;
	case 8:							//coins wan
		m_position.x = (float)0.845*screen.x;
		m_position.y = (float)0.51*screen.y;
		break;
	case 9:							//coins qian
		m_position.x = (float)0.865*screen.x;
		m_position.y = (float)0.51*screen.y;
		break;
	case 10:							//coins bai
		m_position.x = (float)0.885*screen.x;
		m_position.y = (float)0.51*screen.y;
		break;
	case 11:							//coins shi
		m_position.x = (float)0.905*screen.x;
		m_position.y = (float)0.51*screen.y;
		break;
	case 12:							//coins ge
		m_position.x = (float)0.925*screen.x;
		m_position.y = (float)0.51*screen.y;
		break;
	}
	float w = (float)number_texture.width;
	float h = (float)number_texture.height;
	float wr = w * 0.5f;
	float hr = h * 0.5f;
	float width = 50.f;

	vertices[0].position = { -width/2, +hr, 0.f };
	vertices[1].position = { +width/2, +hr, 0.f };
	vertices[2].position = { +width/2, -hr, 0.f };
	vertices[3].position = { -width/2, -hr, 0.f };

	glGenBuffers(1, &mesh.vbo);
	glGenBuffers(1, &mesh.ibo);

	// Vertex Array (Container for Vertex + Index buffer)
	glGenVertexArrays(1, &mesh.vao);
	if (gl_has_errors())
		return false;

	// Loading shaders
	if (!effect.load_from_file(shader_path("textured.vs.glsl"), shader_path("textured.fs.glsl")))
		return false;

	m_scale = { 1.0f,1.0f };

	return true;
}

void Shop_data::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);
    glDeleteBuffers(1, &mesh.ibo);

	effect.release();
}

void Shop_data::draw(const mat3 & projection)
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
		glBindTexture(GL_TEXTURE_2D, number_texture.id);

		// Setting uniform values to the currently bound program
		glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform);
		float color[] = { 1.f, 1.f, 0.f };
		glUniform3fv(color_uloc, 1, color);
		glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);

		// Drawing!
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}

void Shop_data::update_numbers(bool shopping, int number, int item_num)
{
	if (shopping && item_num !=0) {
		get_texture(number);
	}
	else {
		get_texture(10);
	}
}
void Shop_data::update_numbers(bool shopping, int number)
{
	if (shopping) {
		get_texture(number);
	}
}


void Shop_data::get_texture(int loc)
{
	float sw = 50.f;
	float w = 500.f;
	float texture_locs[] = { 0.f, sw / w, 2 * sw / w, 3 * sw / w, 4 * sw / w, 5 * sw / w, 6 * sw / w, 7 * sw / w, 8 * sw / w, 9 * sw / w, 1.f ,1.f};

	vertices[0].texcoord = { texture_locs[loc], 1.f };//top left
	vertices[1].texcoord = { texture_locs[loc + 1], 1.f };//top right
	vertices[2].texcoord = { texture_locs[loc + 1], 0.f };//bottom right
	vertices[3].texcoord = { texture_locs[loc], 0.f };//bottom left

	// counterclockwise as it's the default opengl front winding direction
	uint16_t indices[] = { 0, 3, 1, 1, 3, 2 };

	// Clearing errors
	gl_flush_errors();

	// Vertex Buffer creation
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TexturedVertex) * 4, vertices, GL_STATIC_DRAW);

	// Index Buffer creation
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * 6, indices, GL_STATIC_DRAW);
}

vec2 Shop_data::get_position()const
{
	return m_position;
}
