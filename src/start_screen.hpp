#pragma once
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "common.hpp"

class Startscreen
{
public:
	Startscreen();
	~Startscreen();
	// Creates all the associated render resources and default transform
	bool init(vec2 screen);

	// Releases all associated resources
	void destroy();

	// Renders the water
	void draw();
	bool update();

	bool is_over()const;

private:
	// Window handle
	GLFWwindow* m_window;

	// Screen texture
	// The draw loop first renders to this texture, then it is used for the water shader
	GLuint m_frame_buffer;
	Texture m_screen_tex;


	// Number of fish eaten by the salmon, displayed in the window title
	unsigned int m_points;

	//zoom
	float zoom_factor;



	Mix_Music* m_background_music;
	Mix_Chunk* m_salmon_dead_sound;
	Mix_Chunk* m_salmon_eat_sound;

	// C++ rng
	std::default_random_engine m_rng;
	std::uniform_real_distribution<float> m_dist;
};