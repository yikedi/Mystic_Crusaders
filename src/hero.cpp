// Header
#include "Hero.hpp"

// internal
#include "enemy.hpp"
#include "fish.hpp"
// stlib
#include <vector>
#include <string>
#include <algorithm>
#include <math.h>

Texture Hero::hero_texture;


bool Hero::init()
{
	//std::vector<Vertex> vertices;
	//std::vector<uint16_t> indices;

	// Load shared texture
	if (!hero_texture.is_valid())
	{
		if (!hero_texture.load_from_file(textures_path("hero.png")))
		{
			fprintf(stderr, "Failed to load enemy texture!");
			return false;
		}
	}

	// The position corresponds to the center of the texture
	float wr = hero_texture.width * 0.5f;
	float hr = hero_texture.height * 0.5f;

	TexturedVertex vertices[4];
	vertices[0].position = { -wr, +hr, -0.01f };
	vertices[0].texcoord = { 0.f, 1.f };
	vertices[1].position = { +wr, +hr, -0.01f };
	vertices[1].texcoord = { 1.f, 1.f,  };
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
	m_scale.x = 1.f;
	m_scale.y = 1.f;
	m_is_alive = true;
	m_position = { 50.f, 100.f };
	m_rotation = 0.f;
	m_light_up_countdown_ms = -1.f;
	max_hp = 100.f;
	max_mp = 100.f;
	hp = max_hp;
	mp = max_mp;


	// Setting initial values, scale is negative to make it face the opposite way
	// 1.0 would be as big as the original texture
	set_color({1.0f,1.0f,1.0f});
	m_direction = {0.f,0.f};
	m_light_up = 0;
	advanced = false;

	return true;
}

// Releases all graphics resources
void Hero::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteBuffers(1, &mesh.vao);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);


}

// Called on each frame by World::update()
void Hero::update(float ms)
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

void Hero::draw(const mat3& projection)
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
	glBindTexture(GL_TEXTURE_2D, hero_texture.id);

	// Setting uniform values to the currently bound program
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform);
	float color[] = { 1.f, 1.f, 1.f };
	glUniform3fv(color_uloc, 1, color);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);

	// Drawing!
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}

// Simple bounding box collision check,
bool Hero::collides_with(const Enemy& enemy)
{
	float dx = m_position.x - enemy.get_position().x;
	float dy = m_position.y - enemy.get_position().y;
	float d_sq = dx * dx + dy * dy;
	float other_r = std::max(enemy.get_bounding_box().x, enemy.get_bounding_box().y);
	float my_r = std::max(m_scale.x, m_scale.y);
	float r = std::max(other_r, my_r);

    if (!advanced) { //This is old code
        r *= 0.6f;
        if (d_sq < r * r) {
			take_damage(2.f);
			return true;
		}

    }
    else {	// mesh level collision detection
        r *= 1.0f;
        float top,bottom,left,right;
		// 0.8 is 2*0.4, 0.4 is the scale of enemy, 2 is because I need to devide by 2 to the distance to the center
		// I need 0.4 because enemy.transform would contain do the scale so I need to scale back before transform
		float scale_back = 2.0f;
        top =  -1.f * enemy.get_bounding_box().y / scale_back;
        bottom =  enemy.get_bounding_box().y / scale_back;
        left = -1.f * enemy.get_bounding_box().x / scale_back;
        right = enemy.get_bounding_box().x / scale_back;
		//points before transform
        vec3 p_top,p_left,p_right,p_bottom;

		//could add a list of points to test for our game
        p_top = mul_vec(enemy.transform,{0,top,1});
        p_bottom = mul_vec(enemy.transform,{0,bottom,1});
        p_left = mul_vec(enemy.transform,{left,0,1});
        p_right = mul_vec(enemy.transform,{right,0,1});

        std::vector<vec3> cur_vertices;
        transform_current_vertex(cur_vertices);
        if (d_sq < r * r) {
            return mesh_collision(p_top,cur_vertices) || mesh_collision(p_bottom,cur_vertices)
                   || mesh_collision(p_left,cur_vertices) || mesh_collision(p_right,cur_vertices);
        }

    }

	return false;
}

bool Hero::collides_with(const Fish& fish)
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

vec2 Hero::get_position()const
{
	return m_position;
}

void Hero::move(vec2 off)
{
	m_position.x += off.x; m_position.y += off.y;
}

void Hero::set_rotation(float radians)
{
	m_rotation = radians;
}

bool Hero::is_alive()const
{
	return m_is_alive;
}

// Called when the salmon collides with a enemy
void Hero::kill()
{
	m_is_alive = false;
	vec3 color = {1.0f,0.f,0.f};
	set_color(color);
}

// Called when the salmon collides with a fish
void Hero::light_up()
{
	m_light_up_countdown_ms = 1500.f;
}

void Hero::set_direction(vec2 direction)
{
	m_direction = direction;
}

vec2 Hero::get_direction()
{
	return m_direction;
}

void Hero::transform_current_vertex(std::vector<vec3> &cur_vertices)
{
    for (size_t i = 0; i < vertices.size(); ++i) {
        vec3 old_position = {vertices.at(i).position.x, vertices.at(i).position.y,1};
        vec3 cur_position = mul_vec(transform, old_position);
        cur_vertices.push_back(cur_position);
    }
}

bool Hero::mesh_collision(vec3 ptest,std::vector<vec3> &cur_vertices)
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

void Hero::set_color(vec3 in_color)
{
	float color[3] = {in_color.x,in_color.y,in_color.z};
	memcpy(m_color,color, sizeof(color));
}

void Hero::take_damage(float damage)
{
	change_hp(-1.f * damage);
	if (hp <= 0.5f) {
		m_is_alive = false;
	}
}

void Hero::change_hp(float d_hp)
{
	hp += d_hp;
	hp = std::min(hp,max_hp);
	hp = std::max(0.5f,hp);
}

void Hero::change_mp(float d_mp)
{
	mp += d_mp;
	mp = std::min(mp, max_mp);
	mp = std::max(0.5f, mp);
}

bool Hero::shoot_projectiles(std::vector<Fireball> & hero_projectiles)
{
	//Fish fish;
	Fireball fireball;
	if (fireball.init(m_rotation))
	{
		fireball.set_position(m_position);
		hero_projectiles.emplace_back(fireball);
		return true;
	}
	fprintf(stderr, "Failed to spawn fish");
	return false;

}




