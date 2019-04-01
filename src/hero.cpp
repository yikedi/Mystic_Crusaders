// Header
#include "Hero.hpp"

// internal
#include "enemy_01.hpp"
#include "enemy_02.hpp"
#include "fish.hpp"


// stlib
#include <vector>
#include <string>
#include <algorithm>
#include <stdlib.h>

SpriteSheet Hero::hero_texture;

bool Hero::init(vec2 screen)
{
	//std::vector<Vertex> vertices;
	//std::vector<uint16_t> indices;
    hero_texture.totalTiles = 21; // custom to current sprite sheet
    hero_texture.subWidth = 64; // custom to current sprite sheet
    m_is_alive = false;

	// Load shared texture
	if (!hero_texture.is_valid())
	{
		if (!hero_texture.load_from_file(textures_path("hero_animation.png")))
		{
			fprintf(stderr, "Failed to load hero texture!");
			return false;
		}
	}

    float tileWidth = (float)hero_texture.width / hero_texture.totalTiles;

	// The position corresponds to the center of the texture
	float wr = tileWidth * 0.5f;
	float hr = hero_texture.height * 0.5f;

	texVertices[0].position = { -wr, +hr, -0.01f };
    texVertices[1].position = { +wr, +hr, -0.01f };
	texVertices[2].position = { +wr, -hr, -0.01f };
	texVertices[3].position = { -wr, -hr, -0.01f };

    for (int i = 0; i <= hero_texture.totalTiles; i++) {
        texture_locs.push_back((float)i * hero_texture.subWidth / hero_texture.width);
    }

    glGenBuffers(1, &mesh.vbo);
    glGenBuffers(1, &mesh.ibo);
    setTextureLocs(14);

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
	float altar_offset = 100.f;
	m_position = { screen.x/2 - altar_offset, screen.y/2 };
	m_rotation = 0.f;
	m_light_up_countdown_ms = -1.f;

	// Setting initial values, scale is negative to make it face the opposite way
	// 1.0 would be as big as the original texture
	set_color({1.0f,1.0f,1.0f});
	m_direction = {0.f,0.f};
	m_light_up = 0;
	advanced = false;
	max_hp = 100.f;
	max_mp = 100.f;
	hp = max_hp;
	mp = max_mp;
    ice_arrow_skill.init();
	thunder_skill.init();
	phoenix_skill.init();
	deceleration = 1.0f;
	momentum_factor = 1.0f;
	momentum.x = 0.f;
	momentum.y = 0.f;
	activeSkill = 0;
	level = 0;
	transition_duration = 3000.f;
	isInTransition = false;
	justFinishedTransition = false;
	just_took_damage = false;
	return true;
}

// Releases all graphics resources
void Hero::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);

}

// Called on each frame by World::update()
void Hero::update(float ms)
{
	//momentum first
	m_position.x += momentum.x;
	m_position.y += momentum.y;

	if (momentum.x > 0.5f) {
		momentum.x = std::max(momentum.x - deceleration, 0.f);
	}
	if (momentum.x < -0.5f) {
		momentum.x = std::min(momentum.x + deceleration, 0.f);
	}

	if (momentum.y > 0.5f) {
		momentum.y = std::max(momentum.y - deceleration, 0.f);
	}
	if (momentum.y < -0.5f) {
		momentum.y = std::min(momentum.y + deceleration, 0.f);
	}

	const float SALMON_SPEED = 200.f;
	float step = SALMON_SPEED * (ms / 1000);
    float animSpeed = 0.0f;
	if (m_is_alive)
	{
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// UPDATE SALMON POSITION HERE BASED ON KEY PRESSED (World::on_key())
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		vec2 displacement = {m_direction.x * step, m_direction.y * step};
		move(displacement);

        if (mp < max_mp)
        {
            mp += 0.05;
        }

        // setting player movement state
        if (m_direction.x > 0.0f && m_direction.y == 0.0f) {
            animSpeed = abs(m_direction.x) * 0.025f;
            if (m_moveState != HeroMoveState::RIGHTMOVING) {
                m_moveState = HeroMoveState::RIGHTMOVING;
                m_animTime = 0.0f;
            }
        }
        else if (m_direction.x < 0.0f && m_direction.y == 0.0f){
            animSpeed = abs(m_direction.x) * 0.025f;
            if (m_moveState != HeroMoveState::LEFTMOVING) {
                m_moveState = HeroMoveState::LEFTMOVING;
                m_animTime = 0.0f;
            }
        }
        else if (m_direction.y > 0.0f) {
            animSpeed = abs(m_direction.y) * 0.025f;
            if (m_moveState != HeroMoveState::FRONTMOVING) {
                m_moveState = HeroMoveState::FRONTMOVING;
                m_animTime = 0.0f;
            }
        }
        else if (m_direction.y < 0.0f) {
            animSpeed = abs(m_direction.y) * 0.025f;
            if (m_moveState != HeroMoveState::BACKMOVING) {
                m_moveState = HeroMoveState::BACKMOVING;
                m_animTime = 0.0f;
            }
        }
        else {
            // load standing sprite
            m_moveState = HeroMoveState::STANDING;
            m_animTime = 0.0f;
        }
        m_animTime += animSpeed * 2;

        // setting texture coordinates
        if (m_moveState == HeroMoveState::LEFTMOVING) {
            int currIndex = 15;
            numTiles = 4;
            currIndex += (int)m_animTime % numTiles;
            setTextureLocs(currIndex);
        }
        else if (m_moveState == HeroMoveState::RIGHTMOVING) {
            int currIndex = 8;
            numTiles = 5;
            currIndex += (int)m_animTime % numTiles;
            setTextureLocs(currIndex);
        }
        else if (m_moveState == HeroMoveState::FRONTMOVING) {
            int currIndex = 0;
            numTiles = 4;
            currIndex += (int)m_animTime % numTiles;
            setTextureLocs(currIndex);
        }
        else if (m_moveState == HeroMoveState::BACKMOVING) {
            int currIndex = 4;
            numTiles = 4;
            currIndex += (int)m_animTime % numTiles;
            setTextureLocs(currIndex);
        }
        else {
            int currIndex = 14;
            setTextureLocs(currIndex);
        }
	}
	else
	{
		// If dead we make it face upwards and sink deep down
		int currIndex = 19;
		numTiles = 2;
		currIndex += (int)m_animTime % numTiles;
		setTextureLocs(currIndex);
	}


	if (m_light_up_countdown_ms > 0.f) {
		m_light_up_countdown_ms -= ms;
		m_light_up = 1;
	}
	else
		m_light_up = 0;
}

void Hero::setTextureLocs(int index) {

    texVertices[0].texcoord = { texture_locs[index], 1.f }; //top left
    texVertices[1].texcoord = { texture_locs[index + 1], 1.f }; //top right
    texVertices[2].texcoord = { texture_locs[index + 1], 0.f }; //bottom right
    texVertices[3].texcoord = { texture_locs[index], 0.f }; //bottom left

    // counterclockwise as it's the default opengl front winding direction
    uint16_t indices[] = { 0, 3, 1, 1, 3, 2 };

    // Clearing errors
    gl_flush_errors();

	// Clear memory allocation
    if (m_is_alive) {
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

void Hero::draw(const mat3& projection)
{
	// Transformation code, see Rendering and Transformation in the template specification for more info
	// Incrementally updates transformation matrix, thus ORDER IS IMPORTANT
	vec2 cur_scale = m_scale;
	vec2 cur_pos = m_position;
	float color[] = { 1.f, 1.f, 1.f };
	if (isInTransition) {
		if(clock() - transition_time <= 1500.f) {
			if(clock() - transition_time > 600.f){
				cur_pos.y -= (clock() - transition_time - 600.f) / 1.5f;
			}
			float ratio = std::max(1 - ((clock() - transition_time) / transition_duration), 0.1f);
			cur_scale.x = m_scale.x * ratio;
			color[0] = color[0] + color[0] * (5 - 5 * ratio);
			color[1] = color[1] + color[1] * (5 - 5 * ratio);
			color[2] = color[2] + color[2] * (5 - 5 * ratio);
		} else {
			cur_pos.y -= (3000.f - (clock() - transition_time)) / 1.5f;
			float ratio = std::min(((clock() - transition_time) / transition_duration), 1.f);
			cur_scale.x = m_scale.x * ratio;
			color[0] = color[0] + color[0] * (5 - 5 * ratio);
			color[1] = color[1] + color[1] * (5 - 5 * ratio);
			color[2] = color[2] + color[2] * (5 - 5 * ratio);
		}
	}
	if(just_took_damage) {
		color[0] = 2.f;
		color[1] = 0.1f;
		color[2] = 0.1f;
		just_took_damage = false;
	}
	transform_begin();
	transform_translate(cur_pos);
	// transform_rotate(m_rotation);
	transform_scale(cur_scale);
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
	GLint light_up_uloc = glGetUniformLocation(effect.program, "light_up");

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
	glUniform3fv(color_uloc, 1, color);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);
	glUniform1iv(light_up_uloc, 1, &m_light_up);

	// Drawing!
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

	if (isInTransition && clock() - transition_time > transition_duration) {
		isInTransition = false;
		justFinishedTransition = true;
	}
}

// Simple bounding box collision check,
bool Hero::collides_with(const Enemy_02& enemy)
{
	float dx = m_position.x - enemy.get_position().x;
	float dy = m_position.y - enemy.get_position().y;
	float d_sq = dx * dx + dy * dy;
	float other_r = std::max(enemy.get_bounding_box().x, enemy.get_bounding_box().y);
	vec2 hero_boundary = { std::fabs(m_scale.x) * hero_texture.subWidth/2, std::fabs(m_scale.y) * hero_texture.height/2 };
	float my_r = std::max(hero_boundary.x, hero_boundary.y);
	float r = std::max(other_r, my_r);

    if (!advanced) { //This is old code
        r *= 0.5f;
		if (d_sq < r * r) {
			take_damage(0.3f);
			return true;
		}
    }
    else {	// mesh level collision detection
        r *= 1.0f;
        float top,bottom,left,right;

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

bool Hero::collides_with(Projectile &projectile)
{
	float dx = m_position.x - projectile.get_position().x;
	float dy = m_position.y - projectile.get_position().y;
	float d_sq = dx * dx + dy * dy;
	float other_r = std::max(projectile.get_bounding_box().x, projectile.get_bounding_box().y);
	float my_r = std::max(m_scale.x, m_scale.y);
	float r = std::max(other_r, my_r);
	r *= 1.f;
	if (d_sq < r * r)
			return true;
	return false;

}

bool Hero::collides_with(Vine &vine)
{
	float dx = m_position.x - vine.get_position().x;
	float dy = m_position.y - vine.get_position().y;
	float d_sq = dx * dx + dy * dy;
	float other_r = std::max(vine.get_bounding_box().x, vine.get_bounding_box().y);
	float my_r = std::max(m_scale.x, m_scale.y);
	float r = std::max(other_r, my_r);
	r *= 1.f;
	if (d_sq < r * r)
		return true;
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

// Animation
void Hero::set_moveState(HeroMoveState state)
{
    m_moveState = state;
}

HeroMoveState Hero::get_moveState()
{
    return m_moveState;
}

void Hero::transform_current_vertex(std::vector<vec3> &cur_vertices)
{
    for (size_t i = 0; i < vertices.size(); ++i) {
        vec3 old_position = {vertices.at(i).position.x, vertices.at(i).position.y,1};
        vec3 cur_position = mul_vec(transform, old_position);
        cur_vertices.push_back(cur_position);
    }
}

vec2 Hero::get_bounding_box()
{
	return { std::fabs(m_scale.x) * hero_texture.subWidth, std::fabs(m_scale.y) * hero_texture.height };
}

bool Hero::mesh_collision(vec3 ptest,std::vector<vec3> &cur_vertices)
{

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

int Hero::get_active_skill()
{
	return activeSkill;
}

void Hero::set_color(vec3 in_color)
{
	float color[3] = {in_color.x,in_color.y,in_color.z};
	memcpy(m_color,color, sizeof(color));
}

void Hero::take_damage(float damage)
{
	just_took_damage = true;
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

float Hero::get_hp()
{
	return hp;
}

float Hero::get_mp()
{
    return mp;
}

void Hero::set_position(vec2 position)
{
	m_position = position;
}

vec2 Hero::get_position()
{
	return m_position;
}

bool Hero::shoot_projectiles(std::vector<Projectile*> & hero_projectiles)
{
	Fireball* fireball = new Fireball(m_rotation, 400.f, 20.0f);
	fireball->set_position(m_position);
	hero_projectiles.emplace_back(fireball);
	return true;

}

bool Hero::use_ice_arrow_skill(std::vector<Projectile*> & hero_projectiles)
{
    if (mp > ice_arrow_skill.get_mpcost())
    {
        float mp_cost = ice_arrow_skill.shoot_ice_arrow(hero_projectiles,m_rotation,m_position);
        change_mp(-1 * mp_cost);
        return true;
    }
    return false;

}

bool Hero::use_thunder_skill(std::vector<Thunder*> & thunders, vec2 position)
{
	if (mp > thunder_skill.get_mpcost())
	{
		float mp_cost = thunder_skill.drop_thunder(thunders, position);
		change_mp(-1 * mp_cost);
		return true;
	}
	return false;
}

bool Hero::use_phoenix_skill(std::vector<phoenix*> & phoenix_list)
{
	if (mp > phoenix_skill.get_mpcost())
	{
		float mp_cost = phoenix_skill.create_phoenix(phoenix_list,m_position);
		change_mp(-1 * mp_cost);
		return true;
	}
	return false;
}
void Hero::level_up(int select_skill,int select_upgrade)
{
	if (select_skill == 0)
		ice_arrow_skill.level_up(select_upgrade);
	else if (select_skill == 1)
		thunder_skill.level_up(select_upgrade);
	else if (select_skill == 2)
		phoenix_skill.level_up(select_upgrade);
}

void Hero::levelup()
{
	light_up();
	hp = std::min(max_hp, hp + 10.f);
	level++;
}

void Hero::apply_momentum(vec2 f)
{
	momentum.x += f.x;
	momentum.y += f.y;
}

bool Hero::use_skill(std::vector<Projectile*> & hero_projectiles, std::vector<Thunder*> & thunders, std::vector<phoenix*> &phoenix_list, vec2 position)
{
	bool success;
	switch (activeSkill)
	{
	case ICE_SKILL:
		success = use_ice_arrow_skill(hero_projectiles);
		break;
	case THUNDER_SKILL:
		success = use_thunder_skill(thunders, position);
		break;
	case PHOENIX_SKILL:
		success = use_phoenix_skill(phoenix_list);
	default:
		success = false;
		break;
	}
	return success;
}

void Hero::set_active_skill(int active)
{
	activeSkill = active;
}

void Hero::next_level()
{
	if(!isInTransition) {
		transition_time = clock();
		isInTransition = true;
	}
}