// Header
#include "altar_portal.hpp"
#include <cmath>
#include <algorithm>

SpriteSheet AltarPortal::altar_texture;

bool AltarPortal::init(vec2 screen)
{
	// Load shared texture
	if (!altar_texture.is_valid())
	{
		if (!altar_texture.load_from_file(textures_path("altar.png")))
		{
			fprintf(stderr, "Failed to load altar texture!");
			return false;
		}
	}

	altar_texture.totalTiles = 30; // custom to current sprite sheet
	altar_texture.subWidth = 256; // custom to current sprite sheet

	// The position corresponds to the center of the texture
	float wr = altar_texture.subWidth * 0.5f;
	float hr = altar_texture.height * 0.5f;

	texVertices[0].position = { -wr, +hr, -0.01f };
	//vertices[0].texcoord = { 0.f, 1.f };
	texVertices[1].position = { +wr, +hr, -0.01f };
	//vertices[1].texcoord = { 1.f, 1.f, };
	texVertices[2].position = { +wr, -hr, -0.01f };
	//vertices[2].texcoord = { 1.f, 0.f };
	texVertices[3].position = { -wr, -hr, -0.01f };
	//vertices[3].texcoord = { 0.f, 0.f };

	for (int i = 0; i <= altar_texture.totalTiles; i++) {
		texture_locs.push_back((float)i * altar_texture.subWidth / altar_texture.width);
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

	m_position = { screen.x/2, screen.y/2 };
	m_screen = screen;
	isPortal = false;
	justChangedToPortal = false;

	return true;
}

bool AltarPortal::update(float ms, int points, int max_points) {
	int curidx = 0;
	float animation_speed = 0.05f;
	if(!isPortal) {
		int tiles = 16;
		float ratio = (float)points / (float)max_points;
		curidx = (int)(ratio * (float)tiles);
	} else {
		if(justChangedToPortal){
			curidx = 17;
			animation_time += animation_speed * 2;
			curidx += (int)animation_time % 13;
			if (curidx == 29) {
				justChangedToPortal = false;
			}
		} else {
			curidx = 26;
			animation_time += animation_speed * 2;
			curidx += (int)animation_time % 4;
		}
	}
	setTextureLocs(curidx);
	return true;
}

void AltarPortal::destroy()
{
    glDeleteBuffers(1, &mesh.vbo);
    glDeleteBuffers(1, &mesh.ibo);
	glDeleteVertexArrays(1, &mesh.vao);

    glDeleteShader(effect.vertex);
    glDeleteShader(effect.fragment);
    glDeleteShader(effect.program);
}

vec2 AltarPortal::get_position()
{
    return m_position;
}

void AltarPortal::set_position(vec2 position)
{
    m_position = position;
}

vec2 AltarPortal::get_bounding_box()const
{
	// fabs is to avoid negative scale due to the facing direction
	return { std::fabs(m_scale.x) * altar_texture.subWidth/2, std::fabs(m_scale.y) * altar_texture.subWidth/2 };
}

void AltarPortal::draw(const mat3& projection)
{
	transform_begin();
	transform_translate({m_position.x, m_position.y});
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
	glBindTexture(GL_TEXTURE_2D, altar_texture.id);

	// Setting uniform values to the currently bound program
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform);
	float color[] = {1.f, 1.f, 1.f};
	glUniform3fv(color_uloc, 1, color);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);

	// Drawing!
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}

void AltarPortal::setTextureLocs(int index)
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


bool AltarPortal::collides_with(Projectile &projectile)
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

bool AltarPortal::collides_with(Hero &hero)
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

bool AltarPortal::collides_with(Enemies &enemy)
{
	float dx = m_position.x - enemy.get_position().x;
	float dy = m_position.y - enemy.get_position().y;
	float d_sq = dx * dx + dy * dy;
	float other_r = std::max(enemy.get_bounding_box().x, enemy.get_bounding_box().y);
	float my_r = std::max(get_bounding_box().x, get_bounding_box().y);
	float r = std::max(other_r, my_r);
	r *= 0.8f;
	if (d_sq < r * r)
		return true;
	return false;
}

void AltarPortal::setIsPortal(bool portal)
{
    isPortal = portal;
	if (portal) {
		justChangedToPortal = true;
	}
}

void AltarPortal::killAll(std::vector<Thunder*> & thunders) {
	float duration = 2500.f;
	float damage = 400.f;
	vec2 scale = {16.f, 16.f};
	vec3 color = {0.8f,0.2f,0.2f};
	Thunder* t1 = new Thunder({m_screen.x/2, m_screen.y/2}, duration, damage, scale, color, true);
	thunders.emplace_back(t1);
}

bool AltarPortal::getIsPortal()
{
    return isPortal;
}