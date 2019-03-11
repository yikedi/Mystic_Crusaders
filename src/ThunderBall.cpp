#include "ThunderBall.h"
#include "algorithm"

SpriteSheet ThunderBall::texture;
bool ThunderBall::init(vec2 position,vec2 scale)
{
	// Load shared texture
	if (!texture.is_valid())
	{	
		if (!texture.load_from_file(textures_path("lightning_bottom.png")))
		{
			fprintf(stderr, "Failed to load thunderball texture!");
			return false;
		}
	}

	// The position corresponds to the center of the texture
	texture.totalTiles = 15; // custom to current sprite sheet
	texture.subWidth = 64; // custom to current sprite sheet

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
	m_scale = scale;
	m_rotation = 0;
	m_position = position;
	elapsedTime = 0;
	animation_time = 0.0f;
	first_time = true;

	return true;
}

vec2 ThunderBall::get_position()
{
	return m_position;
}

void ThunderBall::set_position(vec2 position)
{
	m_position = position;
}

void ThunderBall::destroy()
{
	//glDeleteBuffers(1, &mesh.vbo);
	//glDeleteBuffers(1, &mesh.ibo);
	//glDeleteVertexArrays(1, &mesh.vao);
	//effect.release();

	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	//glDeleteBuffers(1, &mesh.vao);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);
}

void ThunderBall::update(float ms) 
{
	//animation

	float animation_speed = 0.1f;
	animation_time += animation_speed * 2;
	int curidx = 0;
	curidx += (int)animation_time % texture.totalTiles;
	setTextureLocs(curidx);
}

void ThunderBall::setTextureLocs(int index)
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

vec2 ThunderBall::get_bounding_box()
{
	return { std::fabs(m_scale.x) * texture.subWidth, std::fabs(m_scale.y) * texture.height };
}

float ThunderBall::get_radius()
{
	return std::max(get_bounding_box().x, get_bounding_box().y);
}

void ThunderBall::draw(const mat3 &projection)
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