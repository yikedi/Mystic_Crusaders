#include "common.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "../ext/stb_image/stb_image.h"

// stlib
#include <iostream>
#include <sstream>

void gl_flush_errors()
{
	while (glGetError() != GL_NO_ERROR);
}

bool gl_has_errors()
{
	GLenum error = glGetError();

	if (error == GL_NO_ERROR) return false;

	while (error != GL_NO_ERROR)
	{
		const char* error_str = "";
		switch (error)
		{
		case GL_INVALID_OPERATION:
			error_str = "INVALID_OPERATION";
			break;
		case GL_INVALID_ENUM:
			error_str = "INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			error_str = "INVALID_VALUE";
			break;
		case GL_OUT_OF_MEMORY:
			error_str = "OUT_OF_MEMORY";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			error_str = "INVALID_FRAMEBUFFER_OPERATION";
			break;
		}

		fprintf(stderr, "OpenGL: %s", error_str);
		error = glGetError();
	}

	return true;
}

float dot(vec2 l, vec2 r)
{
	return l.x * r.x + l.y * r.y;
}

float dot(vec3 l, vec3 r)
{
	return l.x * r.x + l.y * r.y + l.z * r.z;
}

mat3 mul(const mat3 & l, const mat3 & r)
{
	mat3 l_t = { { l.c0.x, l.c1.x, l.c2.x},
	{ l.c0.y, l.c1.y, l.c2.y } ,
	{ l.c0.z, l.c1.z, l.c2.z } };

	mat3 ret;
	ret.c0.x = dot(l_t.c0, r.c0);
	ret.c0.y = dot(l_t.c1, r.c0);
	ret.c0.z = dot(l_t.c2, r.c0);

	ret.c1.x = dot(l_t.c0, r.c1);
	ret.c1.y = dot(l_t.c1, r.c1);
	ret.c1.z = dot(l_t.c2, r.c1);

	ret.c2.x = dot(l_t.c0, r.c2);
	ret.c2.y = dot(l_t.c1, r.c2);
	ret.c2.z = dot(l_t.c2, r.c2);
	return ret;
}

vec3 mul_vec(const mat3 & l, vec3 x)
{
	mat3 A = { { l.c0.x, l.c1.x, l.c2.x},
				 { l.c0.y, l.c1.y, l.c2.y } ,
				 { l.c0.z, l.c1.z, l.c2.z } };

	return {dot(A.c0, x), dot(A.c1, x), dot(A.c2, x)};

}

vec2 normalize(vec2 v)
{
	float m = sqrtf(dot(v, v));
	return { v.x / m, v.y / m };
}

float det(vec2 u, vec2 v)
{
	return u.x * v.y - u.y * v.x;
}

Texture::Texture()
{

}

Texture::~Texture()
{
	if (id != 0) glDeleteTextures(1, &id);
	if (depth_render_buffer_id != 0) glDeleteRenderbuffers(1, &depth_render_buffer_id);
}

bool Texture::load_from_file(const char* path)
{
	if (path == nullptr) 
		return false;
	
	stbi_uc* data = stbi_load(path, &width, &height, NULL, 4);
	if (data == NULL)
		return false;

	gl_flush_errors();
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	stbi_image_free(data);
	return !gl_has_errors();
}

// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
bool Texture::create_from_screen(GLFWwindow const * const window) {
	gl_flush_errors();
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

    glfwGetFramebufferSize(const_cast<GLFWwindow *>(window), &width, &height);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Generate the render buffer with the depth buffer
	glGenRenderbuffers(1, &depth_render_buffer_id);
	glBindRenderbuffer(GL_RENDERBUFFER, depth_render_buffer_id);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_render_buffer_id);

	// Set id as colour attachement #0
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, id, 0);

	// Set the list of draw buffers.
	GLenum draw_buffers[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, draw_buffers); // "1" is the size of DrawBuffers

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;

	return !gl_has_errors();
}

bool Texture::is_valid()const
{
	return id != 0;
}

namespace
{
	bool gl_compile_shader(GLuint shader)
	{
		glCompileShader(shader);
		GLint success = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (success == GL_FALSE)
		{
			GLint log_len;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);
			std::vector<char> log(log_len);
			glGetShaderInfoLog(shader, log_len, &log_len, log.data());
			glDeleteShader(shader);

			fprintf(stderr, "GLSL: %s", log.data());
			return false;
		}

		return true;
	}
}

bool Effect::load_from_file(const char* vs_path, const char* fs_path)
{
	gl_flush_errors();

	// Opening files
	std::ifstream vs_is(vs_path);
	std::ifstream fs_is(fs_path);

	if (!vs_is.good() || !fs_is.good())
	{
		fprintf(stderr, "Failed to load shader files %s, %s", vs_path, fs_path);
		return false;
	}

	// Reading sources
	std::stringstream vs_ss, fs_ss;
	vs_ss << vs_is.rdbuf();
	fs_ss << fs_is.rdbuf();
	std::string vs_str = vs_ss.str();
	std::string fs_str = fs_ss.str();
	const char* vs_src = vs_str.c_str();
	const char* fs_src = fs_str.c_str();
	GLsizei vs_len = (GLsizei)vs_str.size();
	GLsizei fs_len = (GLsizei)fs_str.size();

	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vs_src, &vs_len);
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fs_src, &fs_len);

	// Compiling
	// Shaders already delete if compilation fails
	if (!gl_compile_shader(vertex))
		return false;

	if (!gl_compile_shader(fragment))
	{
		glDeleteShader(vertex);
		return false;
	}

	// Linking
	program = glCreateProgram();
	glAttachShader(program, vertex);
	glAttachShader(program, fragment);
	glLinkProgram(program);
	{
		GLint is_linked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &is_linked);
		if (is_linked == GL_FALSE)
		{
			GLint log_len;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_len);
			std::vector<char> log(log_len);
			glGetProgramInfoLog(program, log_len, &log_len, log.data());

			release();
			fprintf(stderr, "Link error: %s", log.data());
			return false;
		}
	}

	if (gl_has_errors())
	{
		release();
		fprintf(stderr, "OpenGL errors occured while compiling Effect");
		return false;
	}

	return true;
}

void Effect::release()
{
//	glDeleteProgram(program);
//	glDeleteShader(vertex);
//	glDeleteShader(fragment);

	glDetachShader(program, vertex);
	glDeleteShader(vertex);
	//
	glDetachShader(program, fragment);
	glDeleteShader(fragment);
	//
	glDeleteProgram(program);
}

Text::Text()
{

}

Text::~Text()
{

}

bool Text::loadCharacters(const char* ft_path)
{

    if (FT_Init_FreeType(&ft))
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

    if (FT_New_Face(ft, ft_path, 0, &face))
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;

    FT_Set_Pixel_Sizes(face, 0, 48);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

    // Loading shaders
    if (!textEffect.load_from_file(shader_path("text.vs.glsl"), shader_path("text.fs.glsl")))
        return false;
    

    for (GLubyte c = 0; c < 128; c++)
    {
        // Load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;
            return false;
        }
        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Now store character for later use
        Character character = {
            texture,
            {face->glyph->bitmap.width, face->glyph->bitmap.rows},
            {face->glyph->bitmap_left, face->glyph->bitmap_top},
            face->glyph->advance.x
        };
        Characters.insert(std::pair<GLchar, Character>(c, character));
    }

    // clear face and freetype library
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // allocate memory
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TexturedVertex) * 6, NULL, GL_DYNAMIC_DRAW);
    glUseProgram(textEffect.program);
    GLint in_position_loc = glGetAttribLocation(textEffect.program, "in_position");
    GLint in_texcoord_loc = glGetAttribLocation(textEffect.program, "in_texcoord");
    glEnableVertexAttribArray(in_position_loc);
    glEnableVertexAttribArray(in_texcoord_loc);
    glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)0);
    glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)sizeof(vec3));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    

    return true;
}

void Text::RenderText(const mat3& projection, std::string text, GLfloat x, GLfloat y, GLfloat scale, vec3 colors)
{
    glUseProgram(textEffect.program);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glUniformMatrix3fv(glGetUniformLocation(textEffect.program, "projection"), 1, GL_FALSE, (float*)&projection);
    glUniform3f(glGetUniformLocation(textEffect.program, "textColor"), colors.x, colors.y, colors.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(vao);

    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;

        // Update VBO for each character
        TexturedVertex vertices[6];
        vertices[0].position = { xpos, ypos, 1.f };
        vertices[0].texcoord = { 0.f, 0.f };
        vertices[1].position = { xpos, ypos+h, 1.f };
        vertices[1].texcoord = { 0.f, 1.f };
        vertices[2].position = { xpos+w, ypos+h, 1.f };
        vertices[2].texcoord = { 1.f, 1.f };
        vertices[3].position = { xpos, ypos, 1.f };
        vertices[3].texcoord = { 0.f, 0.f };
        vertices[4].position = { xpos+w, ypos+h, 1.f };
        vertices[4].texcoord = { 1.f, 1.f };
        vertices[5].position = { xpos+w, ypos, 1.f };
        vertices[5].texcoord = { 1.f, 0.f };
        
        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderable::transform_begin()
{
	transform = { { 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f}, { 0.f, 0.f, 1.f} };
}

void Renderable::transform_scale(vec2 scale)
{
	mat3 S = { { scale.x, 0.f, 0.f },{ 0.f, scale.y, 0.f },{ 0.f, 0.f, 1.f } };
	transform = mul(transform, S);
}

void Renderable::transform_rotate(float radians)
{
	float c = cosf(radians);
	float s = sinf(radians);
	mat3 R = { { c, s, 0.f },{ -s, c, 0.f },{ 0.f, 0.f, 1.f } };
	transform = mul(transform, R);
}

void Renderable::transform_translate(vec2 offset)
{
	mat3 T = { { 1.f, 0.f, 0.f },{ 0.f, 1.f, 0.f },{ offset.x, offset.y, 1.f } };
	transform = mul(transform, T);
}

void Renderable::transform_end()
{
	//
}

void Renderable::transform_current_vertex(std::vector<vec3> &cur_vertices)
{
	for (size_t i = 0; i < vertices.size(); ++i) {
		vec3 old_position = { vertices.at(i).position.x, vertices.at(i).position.y,1 };
		vec3 cur_position = mul_vec(transform, old_position);
		cur_vertices.push_back(cur_position);
	}
}

bool Renderable::mesh_collision(vec3 ptest, std::vector<vec3> &cur_vertices)
{

	for (size_t i = 0; i < indices.size(); i += 3) {

		//three vertices of a triangle
		vec3 A = cur_vertices.at(indices[i]);
		vec3 B = cur_vertices.at(indices[i + 1]);
		vec3 C = cur_vertices.at(indices[i + 2]);

		/*v0 = C - A
		v1 = B - A
		v2 = P - A*/

		vec2 v0 = { C.x - A.x, C.y - A.y };
		vec2 v1 = { B.x - A.x, B.y - A.y };
		vec2 v2 = { ptest.x - A.x,ptest.y - A.y };

		// Compute dot products
		float dot00 = dot(v0, v0);
		float dot01 = dot(v0, v1);
		float dot02 = dot(v0, v2);
		float dot11 = dot(v1, v1);
		float dot12 = dot(v1, v2);

		// Compute barycentric coordinates
		float invDenom = 1 / (dot00 * dot11 - dot01 * dot01);
		float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
		float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

		if ((u >= 0) && (v >= 0) && (u + v < 1))
			int a = 0;

		return (u >= 0) && (v >= 0) && (u + v < 1);

	}

	return false;
}
