#include "scrollable.hpp"

bool Scrollable::init(vec2 position, vec2 screen, float increment) {

	// Load shared texture
	scroll_texture.load_from_file(textures_path("scrollerTextSemiTransparent.png"));
	float w = screen.x;
	float individual_scale_factor = scroll_texture.width / screen.x;
	float h = scroll_texture.height / individual_scale_factor; // maintains current ratio
	float wr = w * 0.5f;
	float hr = h * 0.5f;
	//width = w;	<- do NOT uncomment these 2 lines! calling things width and height is bad practice
	//height = h;
	scroll_width = scroll_texture.width;
	scroll_height = h;

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
	m_scale = {1.f,1.f};
	m_is_in_use = true;
	m_position = position;
	m_position.y += h / 2.f + 10.f;	// 10.f is buffer
	_increment = increment;

	return true;
}

void Scrollable::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteVertexArrays(1, &mesh.vao);

	effect.release();
}

void Scrollable::draw(const mat3 &projection)
{
	// if ((button_hoverable && mouse_hovering) || !button_hoverable) {
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
	glBindTexture(GL_TEXTURE_2D, scroll_texture.id);

	// Setting uniform values to the currently bound program
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform);
	float color[] = { 1.f, 1.f, 1.f };
	glUniform3fv(color_uloc, 1, color);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);

	// Drawing!
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
	// }
}

void Scrollable::update() {
	// updates the current position for our scrollable
	float y_pos = m_position.y - _increment;
	m_position.y = y_pos;
	// set_position( { m_position.x, y_pos } );
}

bool Scrollable::check_position_for_ending() {
	if ((m_position.y + (scroll_height / 2.f) + 10.f) < 0.f) {
		// Our text has scrolled past the top edge of the screen by 10.f (buffer amount)
		return true;
	}
	else {
		return false;
	}
}