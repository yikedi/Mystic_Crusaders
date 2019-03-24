// Header
#include "treetrunk.hpp"

// internal
#include "hero.hpp"
#include "enemy_01.hpp"
#include "enemy_02.hpp"
#include "fish.hpp"


// stlib
#include <vector>
#include <string>
#include <algorithm>

bool Treetrunk::init(vec2 screen)
{

	// Reads the salmon mesh from a file, which contains a list of vertices and indices
	FILE* mesh_file = fopen(mesh_path("treetrunk.mesh"), "r");
	if (mesh_file == nullptr)
		return false;

	// Reading vertices and colors
	size_t num_vertices;
	fscanf(mesh_file, "%zu\n", &num_vertices);
	for (size_t i = 0; i < num_vertices; ++i)
	{
		float x, y, z;
		float _u[3]; // unused
		int r, g, b;
		fscanf(mesh_file, "%f %f %f %f %f %f %d %d %d\n", &x, &y, &z, _u, _u + 1, _u + 2, &r, &g, &b);
		Vertex vertex;
		vertex.position = { x, y, -z };
		vertex.color = { (float)r / 255, (float)g / 255, (float)b / 255 };
		vertices.push_back(vertex);
	}

	// Reading associated indices
	size_t num_indices;
	fscanf(mesh_file, "%zu\n", &num_indices);
	for (size_t i = 0; i < num_indices; ++i)
	{
		int idx[3];
		fscanf(mesh_file, "%d %d %d\n", idx, idx + 1, idx + 2);
		indices.push_back((uint16_t)idx[0]);
		indices.push_back((uint16_t)idx[1]);
		indices.push_back((uint16_t)idx[2]);
	}

	// Done reading
	fclose(mesh_file);

	// Clearing errors
	gl_flush_errors();

	// Vertex Buffer creation
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

	// Index Buffer creation
	glGenBuffers(1, &mesh.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * indices.size(), indices.data(), GL_STATIC_DRAW);

	// Vertex Array (Container for Vertex + Index buffer)
	glGenVertexArrays(1, &mesh.vao);
	if (gl_has_errors())
		return false;

	// Loading shaders
	if (!effect.load_from_file(shader_path("colored.vs.glsl"), shader_path("colored.fs.glsl")))
		return false;

	// Setting initial values
	m_scale.x = -100.f;
	m_scale.y = 60.f;
	
	m_num_indices = indices.size();
	//m_position = { screen.x / 3, screen.y / 3 };
	m_rotation = 0.f;
	m_color = { 1.f, 1.f, 1.f };

	return true;
}

// Releases all graphics resources
void Treetrunk::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteBuffers(1, &mesh.vao);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);
}

// Called on each frame by World::update()
void Treetrunk::update(float ms)
{
	

}

void Treetrunk::draw(const mat3& projection)
{
	transform_begin();

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// SALMON TRANSFORMATION CODE HERE

	transform_translate(m_position);
	transform_rotate(m_rotation);
	transform_scale(m_scale);


	transform_end();

	// Setting shaders
	glUseProgram(effect.program);

	// Enabling alpha channel for textures
	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);

	// Getting uniform locations
	GLint transform_uloc = glGetUniformLocation(effect.program, "transform");
	GLint color_uloc = glGetUniformLocation(effect.program, "fcolor");
	GLint projection_uloc = glGetUniformLocation(effect.program, "projection");
	GLint light_up_uloc = glGetUniformLocation(effect.program, "light_up");

	// Setting vertices and indices
	glBindVertexArray(mesh.vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);

	// Input data location as in the vertex buffer
	GLint in_position_loc = glGetAttribLocation(effect.program, "in_position");
	GLint in_color_loc = glGetAttribLocation(effect.program, "in_color");
	glEnableVertexAttribArray(in_position_loc);
	glEnableVertexAttribArray(in_color_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(vec3));

	// Setting uniform values to the currently bound program
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform);

	// Tree trunk color
	float color[] = { m_color.x, m_color.y, m_color.z };
	glUniform3fv(color_uloc, 1, color);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);


	// Drawing!
	glDrawElements(GL_TRIANGLES, (GLsizei)m_num_indices, GL_UNSIGNED_SHORT, nullptr);

}



vec2 Treetrunk::get_position()const
{
	return m_position;
}


void Treetrunk::set_position(vec2 position)
{
	m_position = position;
}

bool Treetrunk::collide_with(Hero &hero)
{
	float dx = m_position.x - hero.get_position().x;
	float dy = m_position.y - hero.get_position().y;
	float d_sq = dx * dx + dy * dy;
	float other_r = std::max(hero.get_bounding_box().x, hero.get_bounding_box().y);
	float my_r = std::max(m_scale.x, m_scale.y);
	float r = std::max(other_r, my_r);

	r *= 10.f;
	float top, bottom, left, right;
	float scale_back = 1.0f;
	float content_ratio = 0.95f; // we have extra space at the sides of texture
	float factor = content_ratio * 0.5; // 0.5 because it's half the length and width from boundary to center
	top = -1.f * hero.get_bounding_box().y / scale_back * factor;
	bottom = hero.get_bounding_box().y / scale_back * factor;
	left = -1.f * hero.get_bounding_box().x / scale_back * factor;
	right = hero.get_bounding_box().x / scale_back * factor;
	//points before transform
	vec3 p_top, p_left, p_right, p_bottom,p_top_left,p_top_right,p_bottom_left,p_bottom_right;

	//could add a list of points to test for our game
	p_top = mul_vec(hero.transform, { 0,top,1 });
	p_bottom = mul_vec(hero.transform, { 0,bottom,1 });
	p_left = mul_vec(hero.transform, { left,0,1 });
	p_right = mul_vec(hero.transform, { right,0,1 });
	p_top_left = mul_vec(hero.transform, { left,top,1 });
	p_top_right = mul_vec(hero.transform, { right,top,1 });
	p_bottom_left = mul_vec(hero.transform, { left,bottom,1 });
	p_bottom_right = mul_vec(hero.transform, { right,bottom,1 });

	std::vector<vec3> test_points;
	test_points.emplace_back(p_top);
	test_points.emplace_back(p_bottom);
	test_points.emplace_back(p_left);
	test_points.emplace_back(p_right);
	test_points.emplace_back(p_top_left);
	test_points.emplace_back(p_top_right);
	test_points.emplace_back(p_bottom_left);
	test_points.emplace_back(p_bottom_right);

	std::vector<vec3> cur_vertices;
	transform_current_vertex(cur_vertices);
	if (d_sq < r * r) {
		//bool res =  mesh_collision(p_top, cur_vertices) || mesh_collision(p_bottom, cur_vertices)
		//	|| mesh_collision(p_left, cur_vertices) || mesh_collision(p_right, cur_vertices);

		bool res = false;
		for (auto &test_point : test_points)
		{
			res = res || mesh_collision(test_point, cur_vertices);
		}
		return res;
	}

	return false;
}


bool Treetrunk::collide_with(Projectile &p)
{
	float dx = m_position.x - p.get_position().x;
	float dy = m_position.y - p.get_position().y;
	float d_sq = dx * dx + dy * dy;
	float other_r = std::max(p.get_bounding_box().x, p.get_bounding_box().y);
	float my_r = std::max(m_scale.x, m_scale.y);
	float r = std::max(other_r, my_r);

	r *= 10.f;
	float top, bottom, left, right;
	float scale_back = abs(p.get_scale().x);
	float factor = 0.95 * 0.5; 
	top = -1.f * p.get_bounding_box().y / scale_back * factor;
	bottom = p.get_bounding_box().y / scale_back * factor;
	left = -1.f * p.get_bounding_box().x / scale_back * factor;
	right = p.get_bounding_box().x / scale_back * factor;
	//points before transform
	vec3 p_top, p_left, p_right, p_bottom,p_top_left, p_top_right, p_bottom_left, p_bottom_right;

	//could add a list of points to test for our game
	p_top = mul_vec(p.transform, { 0,top,1 });
	p_bottom = mul_vec(p.transform, { 0,bottom,1 });
	p_left = mul_vec(p.transform, { left,0,1 });
	p_right = mul_vec(p.transform, { right,0,1 });

	p_top_left = mul_vec(p.transform, { left,top,1 });
	p_top_right = mul_vec(p.transform, { right,top,1 });
	p_bottom_left = mul_vec(p.transform, { left,bottom,1 });
	p_bottom_right = mul_vec(p.transform, { right,bottom,1 });

	std::vector<vec3> test_points;
	test_points.emplace_back(p_top);
	test_points.emplace_back(p_bottom);
	test_points.emplace_back(p_left);
	test_points.emplace_back(p_right);
	test_points.emplace_back(p_top_left);
	test_points.emplace_back(p_top_right);
	test_points.emplace_back(p_bottom_left);
	test_points.emplace_back(p_bottom_right);

	std::vector<vec3> cur_vertices;
	transform_current_vertex(cur_vertices);
	if (d_sq < r * r) {
		bool res = false;
		for (auto &test_point : test_points)
		{
			res = res || mesh_collision(test_point, cur_vertices);
		}
		return res;
	}

	return false;
}

bool Treetrunk::collide_with(Enemies &e)
{
	float dx = m_position.x - e.get_position().x;
	float dy = m_position.y - e.get_position().y;
	float d_sq = dx * dx + dy * dy;
	float other_r = std::max(e.get_bounding_box().x, e.get_bounding_box().y);
	float my_r = std::max(m_scale.x, m_scale.y);
	float r = std::max(other_r, my_r);

	r *= 10.f;
	float top, bottom, left, right;
	float scale_back = abs(e.get_scale().x);
	float factor = 0.95 * 0.5;
	top = -1.f * e.get_bounding_box().y / scale_back * factor;
	bottom = e.get_bounding_box().y / scale_back * factor;
	left = -1.f * e.get_bounding_box().x / scale_back * factor;
	right = e.get_bounding_box().x / scale_back * factor;
	//points before transform
	vec3 p_top, p_left, p_right, p_bottom,p_top_left, p_top_right, p_bottom_left, p_bottom_right;

	//could add a list of points to test for our game
	p_top = mul_vec(e.transform, { 0,top,1 });
	p_bottom = mul_vec(e.transform, { 0,bottom,1 });
	p_left = mul_vec(e.transform, { left,0,1 });
	p_right = mul_vec(e.transform, { right,0,1 });

	p_top_left = mul_vec(e.transform, { left,top,1 });
	p_top_right = mul_vec(e.transform, { right,top,1 });
	p_bottom_left = mul_vec(e.transform, { left,bottom,1 });
	p_bottom_right = mul_vec(e.transform, { right,bottom,1 });

	vec3 p_top_left_middle = mul_vec(e.transform, {left + (right - left) * 0.25f,top,1});
	vec3 p_top_right_middle = mul_vec(e.transform, { left + (right - left) * 0.75f,top,1 });
	vec3 p_bottom_left_middle = mul_vec(e.transform, { left + (right - left) * 0.25f,bottom,1 });
	vec3 p_bottom_right_middle = mul_vec(e.transform, { left + (right - left) * 0.75f,bottom,1 });


	std::vector<vec3> test_points;
	test_points.emplace_back(p_top);
	test_points.emplace_back(p_bottom);
	test_points.emplace_back(p_left);
	test_points.emplace_back(p_right);
	test_points.emplace_back(p_top_left);
	test_points.emplace_back(p_top_right);
	test_points.emplace_back(p_bottom_left);
	test_points.emplace_back(p_bottom_right);
	test_points.emplace_back(p_top_left_middle);
	test_points.emplace_back(p_top_right_middle);
	test_points.emplace_back(p_bottom_left_middle);
	test_points.emplace_back(p_bottom_right_middle);


	std::vector<vec3> cur_vertices;
	transform_current_vertex(cur_vertices);
	if (d_sq < r * r) {
		bool res = false;
		for (auto &test_point : test_points)
		{
			res = res || mesh_collision(test_point, cur_vertices);
		}
		return res;
	}

	return false;
}