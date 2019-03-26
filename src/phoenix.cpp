#include "phoenix.h"
#include "FireBall.h"
#include <algorithm>
#include <stdlib.h>


SpriteSheet phoenix::texture;
phoenix::phoenix(float hp, float damage, vec2 position, vec2 scale)
{
	init(hp, damage, position, scale);
}

bool phoenix::init(float hp, float damage, vec2 position, vec2 scale)
{
	if (!texture.is_valid())
	{
		if (!texture.load_from_file(textures_path("phoenix.png")))
		{
			fprintf(stderr, "Failed to load phoenix texture!");
			return false;
		}
	}

	// The position corresponds to the center of the texture
	texture.totalTiles = 15; // custom to current sprite sheet
	texture.subWidth = 128; // custom to current sprite sheet

	// The position corresponds to the center of the texture
	float wr = texture.subWidth * 0.5f;
	float hr = texture.height * 0.5f;

	texVertices[0].position = { -wr, +hr, -0.01f };
	//vertices[0].texcoord = { 0.f, 1.f };
	texVertices[1].position = { +wr, +hr, -0.01f };
	//vertices[1].texcoord = { 1.f, 1.f, };
	texVertices[2].position = { +wr, -hr, -0.01f };
	//vertices[2].texcoord = { 1.f, 0.f };
	texVertices[3].position = { -wr, -hr, -0.01f };
	//vertices[3].texcoord = { 0.f, 0.f };

	for (int i = 0; i <= texture.totalTiles; i++) {
		texture_locs.push_back((float)i * texture.subWidth / texture.width);
	}

	// counterclockwise as it's the default opengl front winding direction
	uint16_t indices[] = { 0, 3, 1, 1, 3, 2 };

	// Clearing errors
	gl_flush_errors();

	// Vertex Buffer creation
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TexturedVertex) * 4, texVertices, GL_STATIC_DRAW);

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

	// Setting initial values, scale is negative to make it face the opposite way
	// 1.0 would be as big as the original texture
	max_hp = hp;
	m_scale = scale;
	m_rotation = 0;
	m_position = position;
	elapsedTime = 0;
	animation_time = 0.0f;
	first_time = true;
	m_hp = hp;
	first_time = true;
	projectile_damage = 20.f;
	particle_damage = 0.02f;


	return true;
}

vec2 phoenix::get_bounding_box()
{
	return { std::fabs(m_scale.x) * texture.subWidth, std::fabs(m_scale.y) * texture.height };
}

vec2 phoenix::get_position()
{
	return m_position;
}

void phoenix::set_position(vec2 position)
{
	m_position = position;
}

void phoenix::shoot_projectiles(vec2 target_position, std::vector<Projectile*> & hero_projectiles)
{
	vec2 shoot_direction = { target_position.x - m_position.x, target_position.y - m_position.y };
	float angle = atan2(shoot_direction.y,shoot_direction.x);
	Fireball* fireball = new Fireball(angle, 400.f, projectile_damage);
	fireball->set_position(m_position);
	hero_projectiles.emplace_back(fireball);
}

void phoenix::attack(std::vector<Enemy_01> &m_enemys_01, std::vector<Enemy_02> &m_enemys_02, std::vector<Enemy_03> &m_enemys_03, std::vector<Projectile*> & hero_projectiles)
{
	vec2 target_position = find_target(m_enemys_01, m_enemys_02, m_enemys_03);
	shoot_projectiles(target_position, hero_projectiles);
}

//find the closest enemy as the target to attack and return the enemies's position
vec2 phoenix::find_target(std::vector<Enemy_01> &m_enemys_01, std::vector<Enemy_02> &m_enemys_02, std::vector<Enemy_03> &m_enemys_03)
{
	float min_distance = std::numeric_limits<float>::infinity();
	vec2 min_location = { 0.f,0.f };
	for (auto & enemy01:m_enemys_01)
	{
		float dx = (m_position.x - enemy01.get_position().x) * (m_position.x - enemy01.get_position().x);
		float dy = (m_position.y - enemy01.get_position().y) * (m_position.y - enemy01.get_position().y);
		float distance = sqrtf(dx + dy);
		if (distance < min_distance)
		{
			min_distance = distance;
			min_location = enemy01.get_position();
		}

	}

	for (auto & enemy : m_enemys_01)
	{
		float dx = (m_position.x - enemy.get_position().x) * (m_position.x - enemy.get_position().x);
		float dy = (m_position.y - enemy.get_position().y) * (m_position.y - enemy.get_position().y);
		float distance = sqrtf(dx + dy);
		if (distance < min_distance)
		{
			min_distance = distance;
			min_location = enemy.get_position();
		}
	}

	for (auto & enemy : m_enemys_02)
	{
		float dx = (m_position.x - enemy.get_position().x) * (m_position.x - enemy.get_position().x);
		float dy = (m_position.y - enemy.get_position().y) * (m_position.y - enemy.get_position().y);
		float distance = sqrtf(dx + dy);
		if (distance < min_distance)
		{
			min_distance = distance;
			min_location = enemy.get_position();
		}
	}

	for (auto & enemy : m_enemys_03)
	{
		float dx = (m_position.x - enemy.get_position().x) * (m_position.x - enemy.get_position().x);
		float dy = (m_position.y - enemy.get_position().y) * (m_position.y - enemy.get_position().y);
		float distance = sqrtf(dx + dy);
		if (distance < min_distance)
		{
			min_distance = distance;
			min_location = enemy.get_position();
		}
	}

	return min_location;

}

void phoenix::destroy()
{

	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);
}

void phoenix::setTextureLocs(int index)
{
	texVertices[0].texcoord = { texture_locs[index], 1.f }; //top left
	texVertices[1].texcoord = { texture_locs[index + 1], 1.f }; //top right
	texVertices[2].texcoord = { texture_locs[index + 1], 0.f }; //bottom right
	texVertices[3].texcoord = { texture_locs[index], 0.f }; //bottom left

	// counterclockwise as it's the default opengl front winding direction
	uint16_t indices[] = { 0, 3, 1, 1, 3, 2 };

	// Clearing errors
	gl_flush_errors();

	// Clear memory allocation
	if (!first_time) {
		destroy();
	}

	// Vertex Buffer creation
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TexturedVertex) * 4, texVertices, GL_STATIC_DRAW);

	// Index Buffer creation
	glGenBuffers(1, &mesh.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * 6, indices, GL_STATIC_DRAW);
	first_time = false;
}

void phoenix::emit_particles() 
{
	int num_particles = 10;
	vec2 range = get_bounding_box();
	float left = m_position.x - range.x / 2;
	float step_to_right = range.x / num_particles;
	for (int i = 0; i < num_particles; i++)
	{
		particles p;
		float lifetime = 1.5f + (rand() % 100) / 100.f; //a slightly different lifetime for each particle 
		float scale = float(rand() % 100) / 1000.f;
		vec2 initial_velocity = { 0.f,-2.f };
		vec2 position = { left + step_to_right, m_position.y };
		p.init(lifetime, scale, position, initial_velocity);
		m_particles.emplace_back();
	}
}

void phoenix::change_hp(float d_hp)
{
	m_hp += d_hp;
	m_hp = std::min(m_hp, max_hp);
	m_hp = std::max(0.5f, m_hp);
}

phoenix::~phoenix()
{
}

void phoenix::draw(const mat3 &projection)
{
	transform_begin();
	transform_translate(m_position);
	transform_rotate(m_rotation);
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
	glBindTexture(GL_TEXTURE_2D, texture.id);

	// Setting uniform values to the currently bound program
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform);
	float color[] = { 1.f, 1.f, 1.f };
	glUniform3fv(color_uloc, 1, color);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);

	// Drawing!
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}