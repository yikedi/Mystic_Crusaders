#include "skill_switch_UI.hpp"

#include <gl3w.h>

#define ICEBLADES 0
#define LIGHTNINGSTORM 1
#define PHOENIX 2
#define TRANSITIONSTEPS 2

SpriteSheet SkillSwitch::skill_texture;

bool SkillSwitch::init(vec2 position)
{
	// Load shared texture
	if (!skill_texture.is_valid())
	{
		if (!skill_texture.load_from_file(textures_path("skill_ui_V2.png")))
		{
			fprintf(stderr, "Failed to load skillswitch texture!");
			return false;
		}
	}

	skill_texture.totalTiles = 6; // custom to current sprite sheet
	skill_texture.subWidth = 300.f; // custom to current sprite sheet
	skill_texture.subHeight = 192.f; // custom to current sprite sheet

	// The position corresponds to the center of the texture
	float wr = skill_texture.subWidth * 0.5f;
	float hr = skill_texture.height * 0.5f;

	texVertices[0].position = { -wr, +hr, -0.02f };
	texVertices[1].position = { +wr, +hr, -0.02f };
	texVertices[2].position = { +wr, -hr, -0.02f };
	texVertices[3].position = { -wr, -hr, -0.02f };
	m_is_in_use = false;

	for (int i = 0; i <= skill_texture.totalTiles; i++) {
		texture_locs.push_back((float)i * skill_texture.subWidth / skill_texture.width);
	}

	glGenBuffers(1, &mesh.vbo);
	glGenBuffers(1, &mesh.ibo);
	setTextureLocs(ICEBLADES);
	currIndex = ICEBLADES;

	// Vertex Array (Container for Vertex + Index buffer)
	glGenVertexArrays(1, &mesh.vao);
	if (gl_has_errors()) {
		fprintf(stderr, "skillswitch glGenVertexArrays has error!");
		return false;
	}

	// Loading shaders
	if (!effect.load_from_file(shader_path("textured.vs.glsl"), shader_path("textured.fs.glsl"))) {
		fprintf(stderr, "skillswitch shaders not loaded!");
		return false;
	}

	// Set properties here! In future
	m_is_in_use = true;
	hasTransitionStarted = false;
	m_scale = { 0.6f, 0.6f };
	zoom_factor = 1.f;
	set_position(position);
	prevSkill = ICEBLADES;

	return true;

}

// Releases all graphics resources
void SkillSwitch::destroy(bool reset)
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);

	if(reset) {
		glDetachShader(effect.program, effect.vertex);
		glDetachShader(effect.program, effect.fragment);
	}
}

void SkillSwitch::draw(const mat3& projection)
{
	transform_begin();
	transform_translate(m_position);
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

	// Setting texVertices and indices
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
	glBindTexture(GL_TEXTURE_2D, skill_texture.id);

	// Setting uniform values to the currently bound program
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform);

	float color[] = { 1.f, 1.f, 1.f };
	glUniform3fv(color_uloc, 1, color);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);

	// Drawing!
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}

void SkillSwitch::update(int skill, float zoom) {

	float animSpeed = 0.2f;
	zoom_factor = zoom;

	if (prevSkill != skill) {
		// Triggered skill change!
		if (!hasTransitionStarted) {
			transitionToSkill = skill;	// Prevents switching over, mid-transition
			if (transitionToSkill > prevSkill) {
				// fprintf(stderr, "triggering transition \n");
				if ((transitionToSkill == PHOENIX) && (prevSkill == ICEBLADES)) { // end of loop, special handling
					movePositiveDirection = false;
				}
				else {
					movePositiveDirection = true;
				}
			}
			else {
				if ((transitionToSkill == ICEBLADES) && (prevSkill == PHOENIX)) {
					movePositiveDirection = true;
				}
				else {
					movePositiveDirection = false;
				}
			}
			hasTransitionStarted = true;
			m_animTime = 0.f;
			tilesLeftToTransition = TRANSITIONSTEPS;	// to make sure we don't go out of bounds
			transitionHelper();
		}
	}
	if (hasTransitionStarted) {
		m_animTime += animSpeed * 2;
		if (m_animTime > 2.f) {
			transitionHelper();
			if (skill != transitionToSkill) {
				// fprintf(stderr, "Mouse wheel travelling too fast! skill %i not equal to skill %i \n", skill, transitionToSkill);
				if (skill == 0) {
					currIndex = 0;
					setTextureLocs(0);
				}
				else if (skill == 1) {
					currIndex = 2;
					setTextureLocs(2);
				}
				else if (skill == 2) {
					currIndex = 4;
					setTextureLocs(4);
				}
			}
			prevSkill = skill;
			hasTransitionStarted = false;
		}
	}


}

void SkillSwitch::transitionHelper() {
	// update animation loop
	if (tilesLeftToTransition) {
		currIndex = currIndex + skill_texture.totalTiles;
		if (movePositiveDirection) {
			currIndex++;	// increment current index with the animation time, but not out of bounds
			tilesLeftToTransition--;	// numTilesTransition is the amount of tiles we have left before we get to final state
		}
		else {
			currIndex--;	// opposite direction
			tilesLeftToTransition--;
		}
		currIndex = currIndex % skill_texture.totalTiles;
	}
	setTextureLocs(currIndex);
}

void SkillSwitch::setTextureLocs(int index) {

	texVertices[0].texcoord = { texture_locs[index], 1.f }; //top left
	texVertices[1].texcoord = { texture_locs[index + 1], 1.f }; //top right
	texVertices[2].texcoord = { texture_locs[index + 1], 0.f }; //bottom right
	texVertices[3].texcoord = { texture_locs[index], 0.f }; //bottom left

	// counterclockwise as it's the default opengl front winding direction
	uint16_t indices[] = { 0, 3, 1, 1, 3, 2 };

	// Clearing errors
	gl_flush_errors();

	// Clear Memory
	if (m_is_in_use) {
		destroy(false);
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

void SkillSwitch::set_position(vec2 position)
{
	m_position = { (position.x + skill_texture.subWidth / 2.f) / zoom_factor, (position.y + skill_texture.subHeight / 2.f) / zoom_factor };
}