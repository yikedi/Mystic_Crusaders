#include "skilltree.hpp"

#include <iostream>

#include <string.h>
#include <cassert>
#include <sstream>
#include <cmath>

#include <gl3w.h>

Texture skilltree::tree_texture;

bool skilltree::init(int level, int score)
{
	if (!tree_texture.is_valid())
	{
		if (!tree_texture.load_from_file(textures_path("skillg")))
		{
			fprintf(stderr, "Failed to load enemy texture!");
			return false;
		}
	}
	float wr = tree_texture.width * 0.5f;
	float hr = tree_texture.height * 0.5f;

	TexturedVertex vertices[4];
	vertices[0].position = { -wr, +hr, -0.02f };
	vertices[0].texcoord = { 0.f, 1.f };
	vertices[1].position = { +wr, +hr, -0.02f };
	vertices[1].texcoord = { 1.f, 1.f };
	vertices[2].position = { +wr, -hr, -0.02f };
	vertices[2].texcoord = { 1.f, 0.f };
	vertices[3].position = { -wr, -hr, -0.02f };
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
	if (gl_has_errors())
		return false;

	// Loading shaders
	if (!effect.load_from_file(shader_path("textured.vs.glsl"), shader_path("textured.fs.glsl")))
		return false;

	return true;
}

void skilltree::destroy()
{
}

void skilltree::draw(const mat3 & projection)
{
	//draw the skilltree when game is paused
	//skipdrawing or being transparent when game is on
}

void skilltree::update_skill(std::string element, int level)
{
	//upadte which skill has been leveled up, pass it to the other file
	//level is related to the skillpoints 
}

bool skilltree::is_paused()
{
	return paused;
}

void skilltree::get_skillpoints(int score)
{
	// updates global variable on the number of free points a player has
	skillpoints = floor((score - used_skillpoints * conversion) / (float) conversion);
}

void skilltree::update_skillpoints(int level)
{
	int consumed_skillpoints = 3 + level * 2;
	fprintf(stderr, "Cost of skillpoints: " + consumed_skillpoints);
	skillpoints -= consumed_skillpoints;
	used_skillpoints += consumed_skillpoints;
	// int consumed_skillpoints = round(pow(level, 2)) * 5;		// deprecated, basically = level^2 * 5
}
