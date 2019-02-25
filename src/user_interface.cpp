// Header
#include "user_interface.hpp"
#include "hero.hpp"

// internal
#include "enemy_01.hpp"
#include "enemy_02.hpp"
#include "fish.hpp"


// stlib
#include <vector>
#include <string>
#include <algorithm>

Texture UserInterface::UserInterface_texture;


bool UserInterface::init(vec2 screen)
{
	//std::vector<Vertex> vertices;
	//std::vector<uint16_t> indices;

	// Load shared texture
	if (!UserInterface_texture.is_valid())
	{
		if (!UserInterface_texture.load_from_file(textures_path("UI-2.bmp")))
		{
			fprintf(stderr, "Failed to load UI texture!");
			return false;
		}
	}

	// The position corresponds to the center of the texture
	vec2 worldSize = screen; // MUST HAVE WORLD SCREEN SIZE PASSED TO IT
	w = worldSize.x;
	h = worldSize.y *(double) 0.25; // scale to an appropriate size for our UI
	float wr = w / (double) 2; // UserInterface_texture.width * 0.5f;
	float hr = h / (double) 2; // UserInterface_texture.height * 0.5f;

	TexturedVertex vertices[4];
	vertices[0].position = { -wr, +hr, -0.01f };
	vertices[0].texcoord = { 0.f, 1.f };
	vertices[1].position = { +wr, +hr, -0.01f };
	vertices[1].texcoord = { 1.f, 1.f, };
	vertices[2].position = { +wr, -hr, -0.01f };
	vertices[2].texcoord = { 1.f, 0.f };
	vertices[3].position = { -wr, -hr, -0.01f };
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

	// Setting initial values
	max_hp = 100.f; // TODO: in later versions, get the max HP from world or sth!
	max_mp = 100.f;
	m_scale.x = 1.f;
	m_scale.y = 1.f;
	m_is_alive = true;
	m_position = { (float) w / 2.f, (float) h * 3.5f };
	m_rotation = 0.f;
	m_light_up_countdown_ms = -1.f;

	// Setting initial values, scale is negative to make it face the opposite way
	// 1.0 would be as big as the original texture
	set_color({ 1.0f,1.0f,1.0f });
	m_direction = { 0.f,0.f };
	m_light_up = 0;
	advanced = false;
	max_hp = 100.f;
	max_mp = 100.f;
	hp = max_hp;
	mp = max_mp;
	return true;
}

// Releases all graphics resources
void UserInterface::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteBuffers(1, &mesh.vao);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);


}

// Called on each frame by World::update()
void UserInterface::update(float ms)
{
	hp = m_hero.get_hp();
	mp = 80.f;
	// mp = m_hero.get_mp(); later when we merge



	if (m_light_up_countdown_ms > 0.f) {
		m_light_up_countdown_ms -= ms;
		m_light_up = 1;
	}
	else
		m_light_up = 0;


}

void UserInterface::draw(const mat3& projection)
{
	// Transformation code, see Rendering and Transformation in the template specification for more info
	// Incrementally updates transformation matrix, thus ORDER IS IMPORTANT
	
	
	transform_begin();
	transform_translate(m_position);
	transform_rotate(m_rotation);
	transform_scale(m_scale);
	transform_end();

	// Setting shaders
	glUseProgram(effect.program);

	// Enabling alpha channel for textures
	glEnable(GL_BLEND); glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
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
	glBindTexture(GL_TEXTURE_2D, UserInterface_texture.id);

	// Setting uniform values to the currently bound program
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform);
	float color[] = { 1.f, 1.f, 1.f };
	glUniform3fv(color_uloc, 1, color);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);

	// Drawing!
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}

void UserInterface::set_rotation(float radians)
{
	m_rotation = radians;
}

// Called when the salmon collides with a fish
void UserInterface::light_up()
{
	m_light_up_countdown_ms = 1500.f;
}

void UserInterface::set_color(vec3 in_color)
{
	float color[3] = { in_color.x,in_color.y,in_color.z };
	memcpy(m_color, color, sizeof(color));
}

void UserInterface::change_hp(float d_hp)
{
	hp += d_hp;
	hp = std::min(hp, max_hp);
	hp = std::max(0.5f, hp);
}

void UserInterface::change_mp(float d_mp)
{
	mp += d_mp;
	mp = std::min(mp, max_mp);
	mp = std::max(0.5f, mp);
}
