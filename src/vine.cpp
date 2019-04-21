// Header
#include "vine.h"

// internal
#include "hero.hpp"
#include "enemy_01.hpp"
#include "enemy_02.hpp"
#include "fish.hpp"
#include "treetrunk.hpp"


// stlib
#include <vector>
#include <string>
#include <algorithm>

SpriteSheet Vine::vine_texture;

bool Vine::init(vec2 screen)
{
	// Load shared texture
	if (!vine_texture.is_valid())
	{
		if (!vine_texture.load_from_file(textures_path("vine.png")))
		{
			fprintf(stderr, "Failed to load enemy texture!");
			return false;
		}
	}

	vine_texture.totalTiles = 8; // custom to current sprite sheet
	vine_texture.subWidth = 64.f; // custom to current sprite sheet
	vine_texture.subHeight = 128.f; // custom to current sprite sheet
	m_is_alive = false;

	// The position corresponds to the center of the texture
	float wr = vine_texture.subWidth * 0.5f;
	float hr = vine_texture.height * 0.5f;

	texVertices[0].position = { -wr, +hr, -0.02f };
	texVertices[1].position = { +wr, +hr, -0.02f };
	texVertices[2].position = { +wr, -hr, -0.02f };
	texVertices[3].position = { -wr, -hr, -0.02f };

	for (int i = 0; i <= vine_texture.totalTiles; i++) {
		texture_locs.push_back((float)i * vine_texture.subWidth / vine_texture.width);
	}

	glGenBuffers(1, &mesh.vbo);
	glGenBuffers(1, &mesh.ibo);
	setTextureLocs(0);

	// Vertex Array (Container for Vertex + Index buffer)
	glGenVertexArrays(1, &mesh.vao);
	if (gl_has_errors())
		return false;

	// Loading shaders
	if (!effect.load_from_file(shader_path("textured.vs.glsl"), shader_path("textured.fs.glsl")))
		return false;

	// Setting initial values, scale is negative to make it face the opposite way
	// 1.0 would be as big as the original texture
	m_scale.x = 0.8f;
	m_scale.y = 0.8f;
	m_rotation = 0.f;
	m_is_alive = true;
	damage = 0.1f;

	return true;
}
void Vine::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteVertexArrays(1, &mesh.vao);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);
}

void Vine::draw(const mat3& projection)
{
	// Transformation code, see Rendering and Transformation in the template specification for more info
	// Incrementally updates transformation matrix, thus ORDER IS IMPORTANT
	transform_begin();
	transform_translate(m_position);
	transform_scale(m_scale);
	transform_end();

	// Setting shaders
	glUseProgram(effect.program);

	// Enabling alpha channel for textures
	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	// Getting uniform locations for glUniform* calls
	GLint transform_uloc = glGetUniformLocation(effect.program, "transform");
	GLint color_uloc = glGetUniformLocation(effect.program, "fcolor");
	GLint projection_uloc = glGetUniformLocation(effect.program, "projection");

	// Setting texVertices and indices
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
	glBindTexture(GL_TEXTURE_2D, vine_texture.id);

	// Setting uniform values to the currently bound program
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform);
	float color[] = { 1.f, 1.f, 1.f };
	glUniform3fv(color_uloc, 1, color);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);

	// Drawing!
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}

void Vine::update(float ms)
{
	float animSpeed = 0.08f;

	// update animation loop
	m_animTime += animSpeed * 2;
	numTiles = 8;
	int currIndex = 0;
	currIndex += (int)m_animTime % numTiles;
	setTextureLocs(currIndex);

}

void Vine::setTextureLocs(int index) {

	texVertices[0].texcoord = { texture_locs[index], 1.f }; //top left
	texVertices[1].texcoord = { texture_locs[index + 1], 1.f }; //top right
	texVertices[2].texcoord = { texture_locs[index + 1], 0.f }; //bottom right
	texVertices[3].texcoord = { texture_locs[index], 0.f }; //bottom left

	// counterclockwise as it's the default opengl front winding direction
	uint16_t indices[] = { 0, 3, 1, 1, 3, 2 };

	// Clearing errors
	gl_flush_errors();

	// Clear Memory

		if(m_is_alive)
		destroy();


	// Vertex Buffer creation
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TexturedVertex) * 4, texVertices, GL_STATIC_DRAW);

	// Index Buffer creation
	glGenBuffers(1, &mesh.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * 6, indices, GL_STATIC_DRAW);

}


vec2 Vine::get_position()const
{
	return m_position;
}


void Vine::set_position(vec2 position)
{
	m_position = position;
}

vec2 Vine::get_bounding_box()
{

	return { std::fabs(0.3f) * vine_texture.subWidth, std::fabs(0.3f) * vine_texture.height };
}

float Vine::get_damage() const { return damage; }