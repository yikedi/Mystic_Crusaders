#include "scrollable.hpp"

bool Scrollable::init(vec2 position, int _width, int _height, float time) {

	// Load shared texture
	if (!scroll_texture.is_valid())
	{
		if (!scroll_texture.load_from_file(textures_path("scrollerText.png")))
		{
			fprintf(stderr, "Failed to load scrollable texture!");
			return false;
		}
	}

	float w = (float) _width;
	float h = (float) _height;
	float wr = w * 0.5f;
	float hr = h * 0.5f;
	halfHeight = hr;
	width = _width;
	height = _height;
	full_height = scroll_texture.height;	// full height of our texture
	total_time = time;

	TexturedVertex vertices[4];
	vertices[0].position = { -wr, +hr, -0.01f };
	vertices[0].texcoord = { 0.f, 1.f };
	vertices[1].position = { +wr, +hr, -0.01f };
	vertices[1].texcoord = { 1.f, 1.f };
	vertices[2].position = { +wr, -hr, -0.01f };
	vertices[2].texcoord = { 1.f, 0.f };
	vertices[3].position = { -wr, -hr, -0.01f };
	vertices[3].texcoord = { 0.f, 0.f };

	// counterclockwise as it's the default opengl front winding direction
	uint16_t indices[] = { 0, 3, 1, 1, 3, 2 };
	m_is_in_use = false;

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

	// Setting initial values
	zoom_factor = 1.f;
	m_scale.x = 1.f;
	m_scale.y = 1.f;
	//set_position({ (float)position.x, (float)position.y });
	//set_position(position);
	m_position = {0.f,0.f};
	m_is_in_use = true;

	return true;
}

void Scrollable::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);
}

void Scrollable::draw(const mat3 &projection)
{
	// if ((button_hoverable && mouse_hovering) || !button_hoverable) {
	gl_flush_errors();

	transform_begin();
	transform_translate(m_position);
	transform_scale(m_scale);
	transform_end();

	// Setting shaders
	glUseProgram(effect.program);

	// Enabling alpha channel for textures
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
	glBindTexture(GL_TEXTURE_2D, scroll_texture.id);

	// Setting uniform values to the currently bound program
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform);

	float color[] = { 1.f, 1.f, 1.f, opacity };
	glUniform4fv(color_uloc, 1, color);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);

	// Drawing!
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
	// }
}

void Scrollable::update(float currTime) {
	/*
	// updates the current position for our scrollable
	current_time = currTime;
	if (!(current_time > total_time)) {
		// we are still updating where the texture is rendered, for the screen
		setTextureLocs(current_time);
	}
	else {
		current_time = total_time;
		setTextureLocs(total_time);
	}
	*/
}

void Scrollable::setTime(float curr_time) {

}

void Scrollable::setTextureLocs(float curr_time) {
	/* 
		Calculations for height: 
		We first have full height. We only need to scroll from the top of the image, to
		just where the top of the screen would be at for the bottom of the image. This 
		distance is traversed over the total time. 
	*/
	// fprintf(stderr, "current time: %f", curr_time);
	fprintf(stderr, " current time global: %f ", current_time);
	float current_height = 1.f - ((full_height - height) * (current_time / total_time)) / full_height;
	float current_bottom = current_height - (height / full_height);
	if (current_height > full_height) {
		fprintf(stderr, "value is too high!");
	}
	if (current_height < 0.f) {
		fprintf(stderr, "value is too low!");
	}

	texVertices[0].texcoord = { 0.f, current_height }; //top left
	texVertices[1].texcoord = { 1.f, current_height }; //top right
	texVertices[2].texcoord = { 0.f, current_bottom }; //bottom right
	texVertices[3].texcoord = { 1.f, current_bottom }; //bottom left

	// counterclockwise as it's the default opengl front winding direction
	uint16_t indices[] = { 0, 3, 1, 1, 3, 2 };

	// Clearing errors
	gl_flush_errors();

	// Clear Memory
	if (m_is_in_use) {
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
}

void Scrollable::set_position(vec2 position)
{
	m_position = { position.x / zoom_factor + (float)width / (2.f * zoom_factor), position.y / zoom_factor + (float)height / (2.f * zoom_factor) };
}