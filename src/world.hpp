#pragma once

// internal
#include "common.hpp"
#include "salmon.hpp"
#include "turtle.hpp"
#include "enemy_01.hpp"
#include "enemy_02.hpp"
#include "fish.hpp"
#include "water.hpp"
#include "hero.hpp"
#include "projectile.h"
#include "fireball.h"
#include "enemy_laser.h"
#include "map_screen.hpp"
#include "start_screen.hpp"
#include "user_interface.hpp"
#include "screen_button.hpp"

// stlib
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
class World
{
public:
	World();
	~World();

	// Creates a window, sets up events and begins the game
	bool init(vec2 screen);

	// Releases all associated resources
	void destroy();

	// Steps the game ahead by ms milliseconds
	bool update(float ms);

	// Renders our scene
	void draw();

	// Should the game be over ?
	bool is_over()const;

	// Returns the size of our game screen
	vec2 getScreenSize();

private:
	// Generates a new enemy
	bool spawn_enemy_01();
	bool spawn_enemy_02();
	bool shootingFireBall;

	// Generates a new fish
	bool spawn_fish();

	// !!! INPUT CALLBACK FUNCTIONS
	void on_key(GLFWwindow*, int key, int, int action, int mod);
	void on_mouse_move(GLFWwindow* window, double xpos, double ypos);
	void on_mouse_click(GLFWwindow* window, int button, int action, int mods);

	void startGame();


private:
	// Window handle
	GLFWwindow* m_window;

	Startscreen start;
	// Screen texture
	// The draw loop first renders to this texture, then it is used for the water shader
	GLuint m_frame_buffer;
	Texture m_screen_tex;
	Mapscreen map;
	// Water effect
	Water m_water;

	// Number of fish eaten by the salmon, displayed in the window title
	unsigned int m_points;
	unsigned int previous_point;

	//zoom
	float zoom_factor;

	//start screen
	bool start_is_over;

	// Game entities
	Hero m_hero;
	std::vector<Enemy_01> m_enemys_01;
	std::vector<Enemy_02> m_enemys_02;
	std::vector<Projectile*> hero_projectiles;
	std::vector<EnemyLaser> enemy_projectiles;
	UserInterface m_interface;

	float m_current_speed;
	float m_next_enemy1_spawn;
	float m_next_enemy2_spawn;
	float m_next_fish_spawn;

	Mix_Music* m_background_music;
	Mix_Chunk* m_salmon_dead_sound;
	Mix_Chunk* m_salmon_eat_sound;

	// C++ rng
	std::default_random_engine m_rng;
	std::uniform_real_distribution<float> m_dist; // default 0..1

	float m_window_width;
	float m_window_height;

	clock_t lastFireProjectileTime;
	Button testButton;
};
