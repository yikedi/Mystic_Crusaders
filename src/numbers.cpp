#include "numbers.hpp"

#include <iostream>

#include <string.h>
#include <cassert>
#include <sstream>
#include <cmath>

#include <gl3w.h>

bool Numbers::init(vec2 screen, int which)
{
	number_texture.load_from_file(textures_path("number.png"));

	switch (which) {
		case 11:							// level
			m_position.x = (float)0.71*screen.x;
			m_position.y = (float)0.42*screen.y;
			break;
		case 12:							// level
			m_position.x = (float)0.76*screen.x;
			m_position.y = (float)0.52*screen.y;
			break;
		case 13:							// map
			m_position.x = (float)0.71*screen.x;
			m_position.y = (float)0.62*screen.y;
			break;
		case 21:							//kill hundred
			m_position.x = (float)0.91*screen.x;
			m_position.y = (float)0.62*screen.y;
			break;
		case 22:							//kill ten
			m_position.x = (float)0.96*screen.x;
			m_position.y = (float)0.72*screen.y;
			break;
		case 23:							//kill
			m_position.x = (float)0.91*screen.x;
			m_position.y = (float)0.82*screen.y;
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

	m_light_up = 0;
	return true;
}

void Numbers::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);
    glDeleteBuffers(1, &mesh.ibo);

    effect.release();
}

void Numbers::draw(const mat3 & projection)
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
		GLint light_up_uloc = glGetUniformLocation(effect.program, "light_up");

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
		float color[] = { 1.f, 1.f, 1.f };
		glUniform3fv(color_uloc, 1, color);
		glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);

		glUniform1iv(light_up_uloc, 1, &m_light_up);
		// Drawing!
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}

void Numbers::update_numbers(bool start_is_over, float num, float world_zoom, vec2 hero_pos)
{
	zoom_factor = world_zoom;
	m_scale = { 1.1f / world_zoom, 1.1f / world_zoom };
	int number = (int)num;
	if (start_is_over) {
		get_texture(number);
	}
}

void Numbers::get_texture(int loc)
{
	float sw = 50.f;
	float w = 500.f;
	float texture_locs[] = { 0.f, sw / w, 2 * sw / w, 3 * sw / w, 4 * sw / w, 5 * sw / w, 6 * sw / w, 7 * sw / w, 8 * sw / w, 9 * sw / w, 1.f };

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

vec2 Numbers::get_position()const
{
	return m_position;
}
void Numbers::set_position(vec2 position, int sh, int offsetx)
{
	m_position = { (position.x + 30.f) / zoom_factor + (float)0 / (2.f * zoom_factor) + offsetx / zoom_factor, position.y / zoom_factor + sh / zoom_factor - (float)51 / (1.f * zoom_factor) };
}