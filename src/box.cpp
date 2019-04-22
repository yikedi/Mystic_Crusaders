// Header
#include "box.hpp"

// internal
#include "hero.hpp"
#include "enemy_01.hpp"
#include "enemy_02.hpp"
#include "fish.hpp"

// stlib
#include <vector>
#include <string>
#include <algorithm>

SpriteSheet Box::box_texture;

bool Box::init(vec2 screen)
{
	// Load shared texture
	if (!box_texture.is_valid())
	{
		if (!box_texture.load_from_file(textures_path("computer.png")))
		{
			fprintf(stderr, "Failed to load computer texture!");
			return false;
		}
	}

	box_texture.totalTiles = 4; // custom to current sprite sheet
	box_texture.subWidth = 210.f; // custom to current sprite sheet
	box_texture.subHeight = 160.f; // custom to current sprite sheet
	m_is_alive = false;

	// The position corresponds to the center of the texture
	float wr = box_texture.subWidth * 0.5f;
	float hr = box_texture.height * 0.5f;

	texVertices[0].position = { -wr, +hr, -0.02f };
	texVertices[1].position = { +wr, +hr, -0.02f };
	texVertices[2].position = { +wr, -hr, -0.02f };
	texVertices[3].position = { -wr, -hr, -0.02f };

	for (int i = 0; i <= box_texture.totalTiles; i++) {
		texture_locs.push_back((float)i * box_texture.subWidth / box_texture.width);
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
	m_scale.x = 0.5f;
	m_scale.y = 0.5f;
	m_rotation = 0.f;
	m_is_alive = true;
	damage = 0.1f;

	return true;
}
void Box::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);
}

void Box::draw(const mat3& projection)
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
	glBindTexture(GL_TEXTURE_2D, box_texture.id);

	// Setting uniform values to the currently bound program
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform);
	float color[] = { 1.f, 1.f, 1.f };
	glUniform3fv(color_uloc, 1, color);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);

	// Drawing!
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}

void Box::update(float ms)
{
	float animSpeed = 0.08f;

	// update animation loop
	m_animTime += animSpeed * 2;
	numTiles = 4;
	int currIndex = 0;
	currIndex += (int)m_animTime % numTiles;
	setTextureLocs(currIndex);

}

void Box::setTextureLocs(int index) {

	texVertices[0].texcoord = { texture_locs[index], 1.f }; //top left
	texVertices[1].texcoord = { texture_locs[index + 1], 1.f }; //top right
	texVertices[2].texcoord = { texture_locs[index + 1], 0.f }; //bottom right
	texVertices[3].texcoord = { texture_locs[index], 0.f }; //bottom left

	// counterclockwise as it's the default opengl front winding direction
	uint16_t indices[] = { 0, 3, 1, 1, 3, 2 };

	// Clearing errors
	gl_flush_errors();

	// Clear Memory

	if (m_is_alive)
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


vec2 Box::get_position()const
{
	return m_position;
}


void Box::set_position(vec2 position)
{
	m_position = position;
}

vec2 Box::get_bounding_box()
{

	return { std::fabs(0.3f) * box_texture.subWidth, std::fabs(0.3f) * box_texture.height };
}

float Box::get_damage() const { return damage; }


bool Box::collides_with(Projectile &projectile)
{
	float dx = m_position.x - projectile.get_position().x;
	float dy = m_position.y - projectile.get_position().y;
	float d_sq = dx * dx + dy * dy;
	float other_r = std::max(projectile.get_bounding_box().x, projectile.get_bounding_box().y);
	float my_r = std::max(get_bounding_box().x, get_bounding_box().y);
	float r = std::max(other_r, my_r);
	r *= 1.f;
	if (d_sq < r * r)
		return true;
	return false;
}

bool Box::collides_with(Hero &hero)
{
	float dx = m_position.x - hero.get_position().x;
	float dy = m_position.y - hero.get_position().y;
	float d_sq = dx * dx + dy * dy;
	float other_r = std::max(hero.get_bounding_box().x, hero.get_bounding_box().y);
	float my_r = std::max(m_scale.x, m_scale.y);
	float r = std::max(other_r, my_r);
	r *= 1.f;
	if (d_sq < r * r)
		return true;
	return false;
}

bool Box::collides_with(Enemies &enemy)
{
	float dx = m_position.x - enemy.get_position().x;
	float dy = m_position.y - enemy.get_position().y;
	float d_sq = dx * dx + dy * dy;
	float other_r = std::max(enemy.get_bounding_box().x, enemy.get_bounding_box().y);
	float my_r = std::max(get_bounding_box().x, get_bounding_box().y);
	float r = std::max(other_r, my_r);
	r *= 1.1f;
	if (d_sq < r * r)
		return true;
	return false;
}