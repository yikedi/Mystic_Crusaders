#include "ice_skill.hpp"

#include <iostream>

#include <string.h>
#include <cassert>
#include <sstream>
#include <cmath>

#include <gl3w.h>
Texture Iceskill::ice_texture;

bool Iceskill::init(vec2 screen)
{
	ice_texture.load_from_file(textures_path("ice_skill_texture.png"));
	float w = ice_texture.width;
	float h = ice_texture.height;
	float wr = w * 0.5f;
	float hr = h * 0.5f;
	float width = 217.f;

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

	m_scale = { 1.0f,1.1f };
	m_position.x = 0.71*screen.x;
	m_position.y = 0.54*screen.y;
	return true;
}

void Iceskill::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteBuffers(1, &mesh.vao);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);
}

void Iceskill::draw(const mat3 & projection)
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
		glBindTexture(GL_TEXTURE_2D, ice_texture.id);

		// Setting uniform values to the currently bound program
		glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform);
		float color[] = { 1.f, 1.f, 1.f };
		glUniform3fv(color_uloc, 1, color);
		glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);

		// Drawing!
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}

void Iceskill::update_ice(bool paused, vec3 ice_num)
{
	float skill1 = ice_num.x;
	float skill2 = ice_num.y;
	float skill3 = ice_num.z;

	if (paused) {
		//fprintf(stderr, "Failed to load enemy texture!");
		if (skill1 ==0.f && skill2 ==0.f && skill3==0.f) {
			fprintf(stderr, "texture!");
			get_texture(0);
		}else if (skill1 == 1.f && skill2 == 0.f && skill3 == 0.f) {
			get_texture(1);
		}else if (skill1 == 2.f && skill2 == 0.f && skill3 == 0.f) {
			get_texture(2);
		}else if (skill1 == 3.f && skill2 == 0.f && skill3 == 0.f) {
			get_texture(3);
		}
	}
}

void Iceskill::get_texture(int loc)
{
	//height/width
	float h = 217.f;
	float w = 1095.f;
	float texture_locs[] = { 0.f, h / w, 2 * h / w, 3 * h / w, 4 * h / w, 5 * h / w };

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

vec2 Iceskill::set_scale(float w, float h, vec2 screen)
{
	return vec2();
}

