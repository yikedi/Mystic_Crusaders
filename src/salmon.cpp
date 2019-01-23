// Header
#include "salmon.hpp"

// internal
#include "turtle.hpp"
#include "fish.hpp"

// stlib
#include <vector>
#include <string>
#include <algorithm>

bool Salmon::init()
{
	//std::vector<Vertex> vertices;
	//std::vector<uint16_t> indices;

	// Reads the salmon mesh from a file, which contains a list of vertices and indices
	FILE* mesh_file = fopen(mesh_path("salmon.mesh"), "r");
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
		fscanf(mesh_file, "%f %f %f %f %f %f %d %d %d\n", &x, &y, &z, _u, _u+1, _u+2, &r, &g, &b);
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
	m_scale.x = -35.f;
	m_scale.y = 35.f;
	m_is_alive = true;
	m_num_indices = indices.size();
	m_position = { 50.f, 100.f };
	m_rotation = 0.f;
	m_light_up_countdown_ms = -1.f;


	//Initialize my variable
	set_color({1.0f,1.0f,1.0f});
	m_direction = {0.f,0.f};
	m_light_up = 0;
	advanced = false;

	return true;
}

// Releases all graphics resources
void Salmon::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteBuffers(1, &mesh.vao);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);


}

// Called on each frame by World::update()
void Salmon::update(float ms)
{
	const float SALMON_SPEED = 200.f;
	float step = SALMON_SPEED * (ms / 1000);
	if (m_is_alive)
	{
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// UPDATE SALMON POSITION HERE BASED ON KEY PRESSED (World::on_key())
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		vec2 displacement = {m_direction.x * step, m_direction.y * step};
		move(displacement);
	}
	else
	{
		// If dead we make it face upwards and sink deep down
		set_rotation(3.1415f);
		move({ 0.f, step });
	}

	if (m_light_up_countdown_ms > 0.f) {
		m_light_up_countdown_ms -= ms;
		m_light_up = 1;
	}
	else
		m_light_up = 0;


}

void Salmon::draw(const mat3& projection)
{
	transform_begin();

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// SALMON TRANSFORMATION CODE HERE

	// see Transformations and Rendering in the specification pdf
	// the following functions are available:
	 transform_translate(m_position);
	 transform_rotate(m_rotation);
	 transform_scale(m_scale);

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// REMOVE THE FOLLOWING LINES BEFORE ADDING ANY TRANSFORMATION CODE
	//transform_translate({ 400.f, 400.f });
	//transform_scale(m_scale);
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

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

	// !!! Salmon Color
	//float color[] = { 1.f, 1.f, 1.f };
	glUniform3fv(color_uloc, 1, m_color);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// HERE TO SET THE CORRECTLY LIGHT UP THE SALMON IF HE HAS EATEN RECENTLY
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//int light_up = 0;
	glUniform1iv(light_up_uloc, 1, &m_light_up);


	// Drawing!
	glDrawElements(GL_TRIANGLES,(GLsizei)m_num_indices, GL_UNSIGNED_SHORT, nullptr);
}

// Simple bounding box collision check, 
bool Salmon::collides_with(const Turtle& turtle)
{
	float dx = m_position.x - turtle.get_position().x;
	float dy = m_position.y - turtle.get_position().y;
	float d_sq = dx * dx + dy * dy;
	float other_r = std::max(turtle.get_bounding_box().x, turtle.get_bounding_box().y);
	float my_r = std::max(m_scale.x, m_scale.y);
	float r = std::max(other_r, my_r);

    if (!advanced) { //This is old code
        r *= 0.6f;
        if (d_sq < r * r)
            return true;
    }
    else {	// mesh level collision detection
        r *= 1.0f;
        float top,bottom,left,right;
		// 0.8 is 2*0.4, 0.4 is the scale of turtle, 2 is because I need to devide by 2 to the distance to the center
		// I need 0.4 because turtle.transform would contain do the scale so I need to scale back before transform
		float scale_back = 0.8f;
        top =  -1.f * turtle.get_bounding_box().y / scale_back;
        bottom =  turtle.get_bounding_box().y / scale_back;
        left = -1.f * turtle.get_bounding_box().x / scale_back;
        right = turtle.get_bounding_box().x / scale_back;
		//points before transform
        vec3 p_top,p_left,p_right,p_bottom;

		//could add a list of points to test for our game
        p_top = mul_vec(turtle.transform,{0,top,1});
        p_bottom = mul_vec(turtle.transform,{0,bottom,1});
        p_left = mul_vec(turtle.transform,{left,0,1});
        p_right = mul_vec(turtle.transform,{right,0,1});

        std::vector<vec3> cur_vertices;
        transform_current_vertex(cur_vertices);
        if (d_sq < r * r) {
            return mesh_collision(p_top,cur_vertices) || mesh_collision(p_bottom,cur_vertices)
                   || mesh_collision(p_left,cur_vertices) || mesh_collision(p_right,cur_vertices);
        }

    }

	return false;
}

bool Salmon::collides_with(const Fish& fish)
{
	float dx = m_position.x - fish.get_position().x;
	float dy = m_position.y - fish.get_position().y;
	float d_sq = dx * dx + dy * dy;
	float other_r = std::max(fish.get_bounding_box().x, fish.get_bounding_box().y);
	float my_r = std::max(m_scale.x, m_scale.y);
	float r = std::max(other_r, my_r);

	if (!advanced) {
		r *= 0.6f;
		if (d_sq < r * r)
			return true;
	}
	else {
		r *= 1.0f;
		float top,bottom,left,right;
		float scale_back = 0.8f;
		top =  -1.f * fish.get_bounding_box().y / scale_back;
		bottom =  fish.get_bounding_box().y / scale_back;
		left = -1.f * fish.get_bounding_box().x / scale_back;
		right = fish.get_bounding_box().x / scale_back;
		//points before transform
		vec3 p_top,p_left,p_right,p_bottom;

		//could add a list of points to test for our game
		p_top = mul_vec(fish.transform,{0,top,1});
		p_bottom = mul_vec(fish.transform,{0,bottom,1});
		p_left = mul_vec(fish.transform,{left,0,1});
		p_right = mul_vec(fish.transform,{right,0,1});

		std::vector<vec3> cur_vertices;
		transform_current_vertex(cur_vertices);
		if (d_sq < r * r) {
			return mesh_collision(p_top,cur_vertices) || mesh_collision(p_bottom,cur_vertices)
				   || mesh_collision(p_left,cur_vertices) || mesh_collision(p_right,cur_vertices);
		}
	}

	return false;
}

vec2 Salmon::get_position()const
{
	return m_position;
}

void Salmon::move(vec2 off)
{
	m_position.x += off.x; m_position.y += off.y;
}

void Salmon::set_rotation(float radians)
{
	m_rotation = radians;
}

bool Salmon::is_alive()const
{
	return m_is_alive;
}

// Called when the salmon collides with a turtle
void Salmon::kill()
{
	m_is_alive = false;
	vec3 color = {1.0f,0.f,0.f};
	set_color(color);
}

// Called when the salmon collides with a fish
void Salmon::light_up()
{
	m_light_up_countdown_ms = 1500.f;
}

void Salmon::set_direction(vec2 direction)
{
	m_direction = direction;
}

vec2 Salmon::get_direction()
{
	return m_direction;
}

void Salmon::transform_current_vertex(std::vector<vec3> &cur_vertices)
{
    for (size_t i = 0; i < vertices.size(); ++i) {
        vec3 old_position = {vertices.at(i).position.x, vertices.at(i).position.y,1};
        vec3 cur_position = mul_vec(transform, old_position);
        cur_vertices.push_back(cur_position);
    }
}

bool Salmon::mesh_collision(vec3 ptest,std::vector<vec3> &cur_vertices)
{
    //mat3 A = mul(m_projection,transform);
//    for (size_t i = 0; i < vertices.size(); ++i) {
//        vec3 cur_position= mul_vec(transform, vertices.at(i).position);
//        cur_vertices.push_back(cur_position);
//    }

    for (size_t i = 0; i < indices.size(); i+=3) {

        //three vertices of a triangle
        vec3 point1 = cur_vertices.at(indices[i]);
        vec3 point2 = cur_vertices.at(indices[i+1]);
        vec3 point3 = cur_vertices.at(indices[i+2]);

        vec2 p1 = {point1.x, point1.y};
        vec2 p1_p2 = {point2.x-point1.x,point2.y-point1.y};
        vec2 p1_p3 = {point3.x-point1.x,point3.y-point1.y};
        vec2 ptest2d = {ptest.x,ptest.y};

        float a,b = 0.f;
        float detv12 = det(p1_p2,p1_p3); //This should not be 0;
        if (detv12 ==0)
            return false;
        float detvv2 = det(ptest2d,p1_p2);
        float detvv3 = det(ptest2d,p1_p3);
        float detv1v2 = det(p1,p1_p2);
        float detv1v3 = det(p1,p1_p3);
        a = (detvv2 - detv1v2) / detv12;
        b = -1.f* (detvv3 - detv1v3) / detv12;

        if (a>0 && b>0 && a+b <1.f) {
            return true;
        }

    }

    return false;
}

void Salmon::set_color(vec3 in_color)
{
	float color[3] = {in_color.x,in_color.y,in_color.z};
	memcpy(m_color,color, sizeof(color));
}



