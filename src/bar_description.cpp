#include "bar_description.hpp"

#include "skilltree.hpp"

#include <iostream>

#include <string.h>
#include <cassert>
#include <sstream>
#include <cmath>

#include <gl3w.h>
Texture Bar_description::hme_texture;


bool Bar_description::init(vec2 screen)
{
	if (!hme_texture.is_valid())
	{
		if (!hme_texture.load_from_file(textures_path("bartext.png")))
		{
			fprintf(stderr, "Failed to load hme texture!");
			return false;
		}
	}

	float w = (float)hme_texture.width;
	float h = (float)hme_texture.height;
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
	float x = screen.x;
	float y = screen.y;

	m_scale = { 1.f, 1.f };
	m_position = { float( x- x/1.1), float(y/1.1) };
	return true;
}

void Bar_description::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);
    glDeleteBuffers(1, &mesh.ibo);

    glDeleteShader(effect.vertex);
    glDeleteShader(effect.fragment);
    glDeleteShader(effect.program);
}

void Bar_description::draw(const mat3 & projection)
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
	glBindTexture(GL_TEXTURE_2D, hme_texture.id);

	// Setting uniform values to the currently bound program
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform);
	float color[] = { 1.f, 1.f, 1.f };
	glUniform3fv(color_uloc, 1, color);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);

	// Drawing!
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}

void Bar_description::update_hme(vec2 hero_pos, float world_zoom, vec2 screen)		//, vec2 hero_position, vec2 screen
{
	float mapx = screen.x;
	float mapy = screen.y;

	float gamex = mapx / world_zoom;
	float gamey = mapy / world_zoom;
	//left-boootom: camerax/2,gamey-cameray/2

	float dx = mapx - gamex;
	float dy = mapy - gamey;
	zoom_factor = world_zoom;
	m_scale = { 0.5f / world_zoom, 0.66f / world_zoom };

}

void Bar_description::set_position(vec2 position, int sh, int offsetx)
{
	m_position = { (position.x + 30.f) / zoom_factor + (float)6 / (2.f * zoom_factor) + offsetx / zoom_factor, position.y / zoom_factor + sh / zoom_factor - (float)60 / (1.f * zoom_factor) };
}

