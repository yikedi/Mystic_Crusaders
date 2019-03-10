// Header
#include "enemy_01.hpp"

#include <cmath>
#include <algorithm>

Texture Enemy_01::enemy_texture;

bool Enemy_01::init(int level)
{
	// Load shared texture
	if (!enemy_texture.is_valid())
	{
		if (!enemy_texture.load_from_file(textures_path("enemy_01.png")))
		{
			fprintf(stderr, "Failed to load enemy texture!");
			return false;
		}
	}

	// The position corresponds to the center of the texture
	float wr = enemy_texture.width * 0.5f;
	float hr = enemy_texture.height * 0.5f;

	TexturedVertex vertices[4];
	vertices[0].position = { -wr, +hr, -0.02f };
	vertices[0].texcoord = { 0.f, 1.f };
	vertices[1].position = { +wr, +hr, -0.02f };
	vertices[1].texcoord = { 1.f, 1.f };
	vertices[2].position = { +wr, -hr, -0.02f };
	vertices[2].texcoord = { 1.f, 0.f };
	vertices[3].position = { -wr, -hr, -0.02f };
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

	// Setting initial values, scale is negative to make it face the opposite way
	// 1.0 would be as big as the original texture
	m_scale.x = 1.0f;
	m_scale.y = 1.0f;
	needFireProjectile = false;
	m_rotation = 0.f;
	enemyRandMoveAngle = 0.f;
	lastFireProjectileTime = clock();
	randMovementTime = clock();
	m_is_alive = true;

	float f = (float)rand() / RAND_MAX;
    float randAttributeFactor = 1.0f + f * (2.0f - 1.0f);

	m_speed = std::min(40.0f + (float)level * 0.5f * randAttributeFactor, 200.0f);
	attackCooldown = std::max(2300.0 - (double)level * 5.0 * randAttributeFactor, 200.0);
	randMovementCooldown = std::max(1000.0 - (double)level * 2.5 * randAttributeFactor, 250.0);
	projectileSpeed = std::min(150.0 + (double)level * 1.0 * randAttributeFactor, 450.0);
	m_range = 50.0 * randAttributeFactor + 475.f;
	hp = std::min(30.0f + (float)level * 0.2f * randAttributeFactor, 50.f);
	deceleration = 1.0f;
	momentum_factor = 1.0f;
	momentum.x = 0.f;
	momentum.y = 0.f;
	m_level = level;
	poweredup = false;

	return true;
}

// Call if init() was successful
// Releases all graphics resources
void Enemy_01::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteVertexArrays(1, &mesh.vao);
	effect.release();
}


void Enemy_01::draw(const mat3& projection)
{
	// Transformation code, see Rendering and Transformation in the template specification for more info
	// Incrementally updates transformation matrix, thus ORDER IS IMPORTANT
	transform_begin();
	transform_translate(m_position);

	if(m_face_left_or_right == 1){
		m_scale.x = -1.0f;
	} else {
		m_scale.x = 1.0f;
	}

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
	glBindTexture(GL_TEXTURE_2D, enemy_texture.id);

	// Setting uniform values to the currently bound program
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform);
	float color[] = { 1.f, 1.f, 1.f };
	if (poweredup){
		switch (powerupType)
		{
			case 0:
				color[0] = 0.2f;
				color[2] = 0.2f;
				break;
			case 1:
				color[1] = 0.2f;
				color[2] = 0.2f;
				break;
			case 2:
				color[0] = 0.8f;
				color[1] = 0.1f;
				color[2] = 0.8f;
				break;
			case 3:
				color[0] = 0.2f;
				color[1] = 0.2f;
				break;
		}
	}
	glUniform3fv(color_uloc, 1, color);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);

	// Drawing!
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}

void Enemy_01::update(float ms, vec2 target_pos)
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

	// Move fish along -X based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.
	float x_diff =  m_position.x - target_pos.x;
	float y_diff =  m_position.y - target_pos.y;
	float distance = std::sqrt(x_diff * x_diff + y_diff * y_diff);
	float enemy_angle = atan2(y_diff, x_diff);
	int facing = 1;
	if (x_diff > 0.0) {
		facing = 0;
	}
	set_facing(facing);
	set_rotation(enemy_angle);
	clock_t currentTime = clock();
	if (distance <= 100.f) {
		needFireProjectile = false;
		float step = m_speed * (ms / 1000);
		m_position.x += cos(enemy_angle)*step;
		m_position.y += sin(enemy_angle)*step;
	} else if (distance <= m_range && checkIfCanFire(currentTime)) {
		needFireProjectile = true;
		setLastFireProjectileTime(currentTime);
		float f = (float)rand() / RAND_MAX;
    	float randAttributeFactor = 1.0f + f * (2.0f - 1.0f);
		m_range = 50.0 * randAttributeFactor + 475.f;
	} else if (distance <= m_range) {
		needFireProjectile = false;
		float step = -m_speed * (ms / 1000);
		m_position.x += cos(enemyRandMoveAngle)*step;
		m_position.y += sin(enemyRandMoveAngle)*step;
	} else {
		needFireProjectile = false;

		float step = -m_speed * (ms / 1000);
		m_position.x += cos(enemy_angle)*step;
		m_position.y += sin(enemy_angle)*step;
	}
	if (checkIfCanChangeDirectionOfMove(currentTime)){
		float LO = enemy_angle - 2.0f;
		float HI = enemy_angle + 2.0f;
		enemyRandMoveAngle = LO + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(HI-LO)));
		setRandMovementTime(currentTime);
	}
}


bool Enemy_01::shoot_projectiles(std::vector<EnemyLaser> & enemy_projectiles)
{
	EnemyLaser enemyLaser;
	float LO = m_rotation - 0.15f * (float) log((m_level/5) + 1);
	float HI = m_rotation + 0.15f * (float) log((m_level/5) + 1);
	if (powerupType == 3) {
		LO = m_rotation - 0.05f * (float) log((m_level/5) + 1);
		HI = m_rotation + 0.05f * (float) log((m_level/5) + 1);

	}
	float fireDir = LO + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(HI-LO)));
	float variation = 0.f;
	if (powerupType == 2) {
		variation = 0.03f;
	}
	if (enemyLaser.init(fireDir, projectileSpeed, 10.0f))
	{
		enemyLaser.set_position(m_position);
		enemyLaser.setVariation(variation);
		enemy_projectiles.emplace_back(enemyLaser);
		return true;
	}
	fprintf(stderr, "Failed to spawn fish");
	return false;

}

bool Enemy_01::checkIfCanFire(clock_t currentClock)
{
	if ((double)(currentClock - lastFireProjectileTime) > attackCooldown) {
		return true;
	}
	return false;
}

void Enemy_01::setLastFireProjectileTime(clock_t c)
{
	lastFireProjectileTime = c;
}

void Enemy_01::powerup()
{
	if (!poweredup) {
		powerupType = 0 + ( std::rand() % ( 3 - 0 + 1 ));
		float f = (float)rand() / RAND_MAX;
		float randAttributeFactor = 1.0f + f * (2.0f - 1.0f);
		switch(powerupType){
			// green
			case 0:
				hp = hp * std::min(1.5f + (float)m_level * 0.05f * randAttributeFactor, 3.5f);
				break;
			// red
			case 1:
				m_speed = m_speed * 1.5f;
				attackCooldown = attackCooldown / 2;
				break;
			// purple
			case 2:
				break;
			// blue
			case 3:
				m_range = m_range * 1.3f;
				projectileSpeed = projectileSpeed * 1.5f;
		}
		poweredup = true;
	}
}