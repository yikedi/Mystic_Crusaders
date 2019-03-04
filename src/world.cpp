// Header
#include "world.hpp"

// stlib
#include <string.h>
#include <cassert>
#include <sstream>

#include <gl3w.h>

// Same as static in c, local to compilation unit
namespace
{
	const int INIT_MAX_ENEMIES = 3;
	const int INIT_MAX_ENEMY_03 = 1;
	size_t MAX_ENEMIES_01 = INIT_MAX_ENEMIES;
	size_t MAX_ENEMIES_02 = INIT_MAX_ENEMIES;
	size_t MAX_ENEMIES_03 = INIT_MAX_ENEMY_03;
	const size_t ENEMY_DELAY_MS = 2000;
	const size_t ENEMY_03_DELAY_MS = 5000;
	float screen_left = 0.f;
	float screen_right = 100.f;
	float screen_top = 0.f;
	float screen_bottom = 100.f;	// called screen_bottom to avoid ambiguity
	float our_x = 0.f;
	float our_y = 0.f;
	// bool in_main_game = false;
	//int stage = 0;

	namespace
	{
		void glfw_err_cb(int error, const char* desc)
		{
			fprintf(stderr, "%d: %s", error, desc);
		}
	}
}

World::World() :
	m_points(0),
	previous_point(0),
	m_next_enemy1_spawn(0.f),
	m_next_enemy2_spawn(1.f),
	m_next_enemy3_spawn(2.f),
	m_next_fish_spawn(0.f)
{
	// Seeding rng with random device

	m_rng = std::default_random_engine(std::random_device()());
}

World::~World()
{

}

// World initialization
bool World::init(vec2 screen)
{
	//-------------------------------------------------------------------------
	// GLFW / OGL Initialization
	// Core Opengl 3.

	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, 0);
	m_window = glfwCreateWindow((int)screen.x, (int)screen.y, "Mystic Crusaders", glfwGetPrimaryMonitor(), nullptr);
	if (m_window == nullptr)
		return false;

	glfwMakeContextCurrent(m_window);
	glfwSwapInterval(1); // vsync

	// Load OpenGL function pointers
	gl3w_init();

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(m_window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((World*)glfwGetWindowUserPointer(wnd))->on_key(wnd, _0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((World*)glfwGetWindowUserPointer(wnd))->on_mouse_move(wnd, _0, _1); };
	//auto reshape_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((World*)glfwGetWindowUserPointer(wnd))->on_mouse_move(wnd, _0, _1); };
	auto mouse_button_callback =[](GLFWwindow* wnd, int _0, int _1, int _2) {((World*)glfwGetWindowUserPointer(wnd))->on_mouse_click(wnd, _0, _1, _2);};

	glfwSetKeyCallback(m_window, key_redirect);
	glfwSetCursorPosCallback(m_window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(m_window, mouse_button_callback);

	//glutReshapeFunc(reshape);

	// Create a frame buffer
	m_frame_buffer = 0;
	glGenFramebuffers(1, &m_frame_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_frame_buffer);

	// Initialize the screen texture
	m_screen_tex.create_from_screen(m_window);

	//-------------------------------------------------------------------------
	// Loading music and sounds
	if (SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		fprintf(stderr, "Failed to initialize SDL Audio");
		return false;
	}

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1)
	{
		fprintf(stderr, "Failed to open audio device");
		return false;
	}

	m_background_music = Mix_LoadMUS(audio_path("music.wav"));
	m_salmon_dead_sound = Mix_LoadWAV(audio_path("salmon_dead.wav"));
	m_salmon_eat_sound = Mix_LoadWAV(audio_path("salmon_eat.wav"));

	if (m_background_music == nullptr || m_salmon_dead_sound == nullptr || m_salmon_eat_sound == nullptr)
	{
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
			audio_path("music.wav"),
			audio_path("salmon_dead.wav"),
			audio_path("salmon_eat.wav"));
		return false;
	}

	// Playing background music undefinitely
	Mix_PlayMusic(m_background_music, -1);

	fprintf(stderr, "Loaded music\n");

	m_current_speed = 1.f;
	zoom_factor = 1.f;
	start_is_over = start.is_over();
	m_window_width = screen.x;
	m_window_height = screen.y;
	m_hero.init(screen);
	m_interface.init({ 300.f, 50.f });
	shootingFireBall = false;
	return start.init(screen) && m_water.init();
	//m_hero.init(screen) && m_water.init();

}

// Releases all the associated resources
void World::destroy()
{
	glDeleteFramebuffers(1, &m_frame_buffer);

	if (m_background_music != nullptr)
		Mix_FreeMusic(m_background_music);
	if (m_salmon_dead_sound != nullptr)
		Mix_FreeChunk(m_salmon_dead_sound);
	if (m_salmon_eat_sound != nullptr)
		Mix_FreeChunk(m_salmon_eat_sound);

	Mix_CloseAudio();

	m_hero.destroy();
	for (auto& enemy : m_enemys_01)
		enemy.destroy();
	for (auto& enemy : m_enemys_02)
		enemy.destroy();
	for (auto& enemy : m_enemys_03)
		enemy.destroy();
	for (auto& h_proj : hero_projectiles)
		h_proj->destroy();
	for (auto& e_proj : enemy_projectiles)
		e_proj.destroy();
	m_enemys_01.clear();
	m_enemys_02.clear();
	hero_projectiles.clear();
	enemy_projectiles.clear();
	// in_main_game = false;
	m_interface.destroy();
	start.destroy();
	glfwDestroyWindow(m_window);
}

// Update our game world
bool World::update(float elapsed_ms)
{
	int w, h;
        glfwGetFramebufferSize(m_window, &w, &h);
	vec2 screen = { (float)w, (float)h };


	start.update(start_is_over);
	if (start_is_over) {
		if (m_hero.is_alive()) {

		if (shootingFireBall && clock() - lastFireProjectileTime > 300) {
			m_hero.shoot_projectiles(hero_projectiles);
			lastFireProjectileTime = clock();
		}

		// Checking hero - Enemy collisions
		for (const auto& enemy : m_enemys_02)
		{
			if (m_hero.collides_with(enemy))
			{
				if (!m_hero.is_alive()) {
					Mix_PlayChannel(-1, m_salmon_dead_sound, 0);
					m_water.set_salmon_dead();
					m_hero.kill();
					break;
				}
			}
		}

		// Checking hero - Enemy collisions
		auto e_proj = enemy_projectiles.begin();
		while (e_proj != enemy_projectiles.end())
		{
			if (m_hero.collides_with(*e_proj))
			{
				m_hero.take_damage(e_proj->get_damage());
                //comment back later
				//e_proj->destroy();
				e_proj = enemy_projectiles.erase(e_proj);
				if (!m_hero.is_alive()) {
					Mix_PlayChannel(-1, m_salmon_dead_sound, 0);
					m_water.set_salmon_dead();
					m_hero.kill();
				}
				break;
			}
			++e_proj;
		}

		if (m_hero.get_position().y > m_window_height - m_hero.m_scale.y * 2) {
			vec2 force = {0.f, -10.f};
			m_hero.apply_momentum(force);
		} else if (m_hero.get_position().y < m_hero.m_scale.y * 2) {
			vec2 force = {0.f, 10.f};
			m_hero.apply_momentum(force);
		}

		if (m_hero.get_position().x > m_window_width - m_hero.m_scale.x * 2) {
			vec2 force = {-10.f, 0.f};
			m_hero.apply_momentum(force);
		} else if (m_hero.get_position().x < m_hero.m_scale.x * 2) {
			vec2 force = {10.f, 0.f};
			m_hero.apply_momentum(force);
		}

		if (m_points - previous_point > 20)
		{
			previous_point = m_points;
			m_hero.level_up();
		}
		}



		for (Enemy_01 enemy : m_enemys_01)
		{
			if (enemy.needFireProjectile == true)
			{
				enemy.shoot_projectiles(enemy_projectiles);
			}
		}

		for (Enemy_03 enemy : m_enemys_03)
		{
			if (enemy.needFireProjectile == true)
			{
				int rand_factor = 0 + ( std::rand() % ( 1 - 0 + 1 ) );
				if (rand_factor == 0)
				{
					if (m_enemys_01.size() > 0) {
						int factor = m_enemys_01.size();
						if (factor == 0) {
							m_enemys_01[0].powerup();
						} else {
							rand_factor = std::rand() % factor + 0;
							m_enemys_01[rand_factor].powerup();
						}
					}
				} else {
					if (m_enemys_02.size() > 0) {
						int factor = m_enemys_02.size();
						if (factor == 0) {
							m_enemys_02[0].powerup();
						} else {
							rand_factor = std::rand() % factor + 0;
							m_enemys_02[rand_factor].powerup();
						}
					}
				}
			}
		}

		// Updating all entities, making the enemy and fish
		// faster based on current
		m_hero.update(elapsed_ms);
		for (auto& enemy : m_enemys_01)
			enemy.update(elapsed_ms * m_current_speed, m_hero.get_position());
		for (auto& enemy : m_enemys_02)
			enemy.update(elapsed_ms * m_current_speed, m_hero.get_position());
		for (auto& enemy : m_enemys_03)
			enemy.update(elapsed_ms * m_current_speed, m_hero.get_position());
		for (auto& h_proj : hero_projectiles)
			h_proj->update(elapsed_ms * m_current_speed);
		for (auto& e_proj : enemy_projectiles)
			e_proj.update(elapsed_ms * m_current_speed);
		m_interface.update({ m_hero.get_hp(), m_hero.get_mp() }, zoom_factor);

		//remove out of screen fireball

		int len = (int) hero_projectiles.size() - 1;
		for (int i = len; i >= 0; i--)
		{
			Projectile* h_proj= hero_projectiles.at(i);
			float w = h_proj->get_bounding_box().x / 2;
			if (h_proj->get_position().x + w < 0.f)
			{
                //h_proj->destroy();
				hero_projectiles.erase(hero_projectiles.begin() + i);
				continue;
			}
		}

		//remove out of screen lasers
		auto e_proj = enemy_projectiles.begin();
		while (e_proj != enemy_projectiles.end())
		{
			float w = e_proj->get_bounding_box().x / 2;
			if (e_proj->get_position().x + w < 0.f)
			{
                //e_proj->destroy();
				e_proj = enemy_projectiles.erase(e_proj);
				continue;
			}

			++e_proj;
		}

		auto enemy = m_enemys_01.begin();

		while (enemy != m_enemys_01.end())
		{
			int len = (int) hero_projectiles.size() - 1;
			for (int i = len; i >= 0; i--)
			{
				Projectile* h_proj= hero_projectiles.at(i);
				if (enemy->collide_with(*h_proj))
				{
					enemy->take_damage(h_proj->get_damage(), h_proj->get_velocity());
					//h_proj->destroy();
					hero_projectiles.erase(hero_projectiles.begin() + i);
					if(!enemy->is_alive()) {
                        //enemy->destroy();
						enemy = m_enemys_01.erase(enemy);
						++m_points;
						MAX_ENEMIES_01 = INIT_MAX_ENEMIES + m_points / 17;
					}
					break;
				}

			}

			if (enemy == m_enemys_01.end() || m_enemys_01.size() == 0){
				break;
			}
			++enemy;
		}

		auto enemy2 = m_enemys_02.begin();

		while (enemy2 != m_enemys_02.end())
		{
			int len = (int) hero_projectiles.size() - 1;
			for (int i = len; i >= 0; i--)
			{
				Projectile* h_proj= hero_projectiles.at(i);
				if (enemy2->collide_with(*h_proj))
				{
					enemy2->take_damage(h_proj->get_damage(), h_proj->get_velocity());
                    //h_proj->destroy();
					hero_projectiles.erase(hero_projectiles.begin() + i);
					if(!enemy2->is_alive()) {
                        //enemy2->destroy();
						enemy2 = m_enemys_02.erase(enemy2);
						++m_points;
						MAX_ENEMIES_02 = INIT_MAX_ENEMIES + m_points / 13;
					}
					break;
				}
			}
			if (enemy2 == m_enemys_02.end() || m_enemys_02.size() == 0){
				break;
			}
			++enemy2;
		}

		auto enemy3 = m_enemys_03.begin();

		while (enemy3 != m_enemys_03.end())
		{
			int len = (int) hero_projectiles.size() - 1;
			for (int i = len; i >= 0; i--)
			{
				Projectile* h_proj= hero_projectiles.at(i);
				if (enemy3->collide_with(*h_proj))
				{
					enemy3->take_damage(h_proj->get_damage(), h_proj->get_velocity());
                    //h_proj->destroy();
					hero_projectiles.erase(hero_projectiles.begin() + i);
					if(!enemy3->is_alive()) {
                        //enemy2->destroy();
						enemy3 = m_enemys_03.erase(enemy3);
						++m_points;
						MAX_ENEMIES_03 = INIT_MAX_ENEMY_03 + m_points / 31;
					}
					break;
				}
			}
			if (enemy3 == m_enemys_03.end() || m_enemys_03.size() == 0){
				break;
			}
			++enemy3;
		}


		// Spawning new enemys
		m_next_enemy1_spawn -= elapsed_ms * m_current_speed;
		if (m_enemys_01.size() < MAX_ENEMIES_01 && m_next_enemy1_spawn < 0.f)
		{
			if (!spawn_enemy_01())
				return false;

			Enemy_01& new_enemy = m_enemys_01.back();

			int left_or_right_spawn = rand() % 2;

			float screen_x = 0;

			if(left_or_right_spawn == 0){
				screen_x = screen.x + 150.f;
			}

			// Setting random initial position
			new_enemy.set_position({ screen_x, 50 + m_dist(m_rng) * (screen.y - 100) });

			// Next spawn
			m_next_enemy1_spawn = (ENEMY_DELAY_MS) + m_dist(m_rng) * (ENEMY_DELAY_MS) - log(m_points + 1) * 200;
		}
		m_next_enemy2_spawn -= elapsed_ms * m_current_speed;
		if (m_enemys_02.size() < MAX_ENEMIES_02 && m_next_enemy2_spawn < 0.f)
		{
			if (!spawn_enemy_02())
				return false;

			Enemy_02& new_enemy = m_enemys_02.back();

			int left_or_right_spawn = rand() % 2;

			float screen_x = 0;

			if(left_or_right_spawn == 0){
				screen_x = screen.x + 150.f;
			}

			// Setting random initial position
			new_enemy.set_position({ screen_x, 50 + m_dist(m_rng) * (screen.y - 100) });

			// Next spawn
			m_next_enemy2_spawn = (ENEMY_DELAY_MS) + m_dist(m_rng) * (ENEMY_DELAY_MS) - log(m_points + 1) * 200;
		}

		m_next_enemy3_spawn -= elapsed_ms * m_current_speed;
		if (m_enemys_03.size() < MAX_ENEMIES_03 && m_next_enemy3_spawn < 0.f && m_points > 20)
		{
			if (!spawn_enemy_03())
				return false;

			Enemy_03& new_enemy = m_enemys_03.back();

			int left_or_right_spawn = rand() % 2;

			float screen_x = 0;

			if(left_or_right_spawn == 0){
				screen_x = screen.x + 150.f;
			}

			// Setting random initial position
			new_enemy.set_position({ screen_x, 50 + m_dist(m_rng) * (screen.y - 100) });

			// Next spawn
			m_next_enemy3_spawn = (ENEMY_03_DELAY_MS) + m_dist(m_rng) * (ENEMY_03_DELAY_MS) - log(m_points + 1) * 200;
		}
	}

	// If hero is dead, restart the game after the fading animation
	if (!m_hero.is_alive() &&
		m_water.get_salmon_dead_time() > 5) {
		int w, h;
		glfwGetWindowSize(m_window, &w, &h);
		m_hero.destroy();
		m_hero.init(screen);
		start.init(screen);
		m_enemys_01.clear();
		m_enemys_02.clear();
		m_enemys_03.clear();
		hero_projectiles.clear();
		enemy_projectiles.clear();
		// in_main_game = false;
		m_interface.destroy();
		m_interface.init({ 300.f, 50.f });
		m_water.reset_salmon_dead_time();
		m_current_speed = 1.f;
		zoom_factor = 1.f;
		m_points = 0;
		map.set_is_over(true);
		start_is_over = false;
	}


	return true;
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void World::draw()
{
	// Clearing error buffer
	gl_flush_errors();

	// Getting size of window
	int w, h;
    glfwGetFramebufferSize(m_window, &w, &h);

	// Updating window title with points
	std::stringstream title_ss;
	title_ss << "Points: " << m_points << " HP:" << m_hero.get_hp() << "MP:" <<m_hero.get_mp();
	glfwSetWindowTitle(m_window, title_ss.str().c_str());

	/////////////////////////////////////
	// First render to the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, m_frame_buffer);

	// Clearing backbuffer
	glViewport(0, 0, w, h);
	glDepthRange(0.00001, 10);
	const float clear_color[3] = { 0.3f, 0.3f, 0.4f };
	glClearColor(clear_color[0], clear_color[1], clear_color[2], 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Fake projection matrix, scales with respect to window coordinates
	// PS: 1.f / w in [1][1] is correct.. do you know why ? (:
	//float screen_left = 0.f;// *-0.5;
	//float screen_top = 0.f;// (float)h * -0.5;
	//float screen_right = (float)w;// *0.5;
	//float bottom = (float)h;// *0.5;

	float sx = zoom_factor * 2.f / (screen_right - screen_left);
	float sy = zoom_factor * 2.f / (screen_top - screen_bottom);	// named "screen_bottom" now because compiler complained about ambiguity

	vec2 salmon_position = m_hero.get_position(); //get the hero position
	our_x = salmon_position.x;
	our_y = salmon_position.y;

	float w_not_scaled = (float)w;
	float h_not_scaled = (float)h;
	float w_scaled = (float)w * zoom_factor;
	float h_scaled = (float)h * zoom_factor;
	screen_left = our_x * zoom_factor - (w_not_scaled / 2); // divided by 2? // in your case this would be x - 400

	//if conditions makes sure that the camera stays in the scene if player reaches the boundary
	if (screen_left < m_hero.m_scale.x * 2) {
		screen_left = m_hero.m_scale.x * 2;
	}
	else if (screen_left + w_not_scaled > w_scaled - m_hero.m_scale.x * 2) {
		screen_left = w_scaled - w_not_scaled - m_hero.m_scale.x * 2;
	}
	screen_top = our_y * zoom_factor - (h_not_scaled / 2); // divided by 2? // and this would be y - 300
	if (screen_top < m_hero.m_scale.y * 2 * zoom_factor) {
		screen_top = m_hero.m_scale.y * 2 * zoom_factor;
	}
	else if (screen_top + h_not_scaled > h_scaled - m_hero.m_scale.y * 2) {
		screen_top = h_scaled - h_not_scaled - m_hero.m_scale.y * 2;
	}
	screen_right = screen_left + w_not_scaled;
	screen_bottom = screen_top + h_not_scaled;

	// for our UI bar
	m_interface.set_position({ screen_left, screen_top });

	float tx = -1 * (screen_right + screen_left) / (screen_right - screen_left);
	float ty = -1 * (screen_top + screen_bottom) / (screen_top - screen_bottom);

	mat3 scaling_2D{ { sx, 0.f, 0.f },{ 0.f, sy, 0.f },{ 0.f, 0.f, 1.f} };
	mat3 translate_2D{ { 1.f, 0.f, 0.f },{ 0.f, 1.f, 0.f },{ tx, ty, 1.f} };

	mat3 projection_2D = mul(translate_2D, scaling_2D);

	start.draw(projection_2D);
	// Drawing entities
	map.draw(projection_2D);
	for (auto& enemy : m_enemys_01)
		enemy.draw(projection_2D);
	for (auto& enemy : m_enemys_02)
		enemy.draw(projection_2D);
	for (auto& enemy : m_enemys_03)
		enemy.draw(projection_2D);
	for (auto& h_proj : hero_projectiles)
		h_proj->draw(projection_2D);
	for (auto& e_proj : enemy_projectiles)
		e_proj.draw(projection_2D);
	m_hero.draw(projection_2D);

	// Testing TODO
	if (start_is_over) {
		m_interface.draw(projection_2D);
	}

	/////////////////////
	// Truely render to the screen
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Clearing backbuffer
	glViewport(0, 0, w, h);
	glDepthRange(0, 10);
	glClearColor(0, 0, 0, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_screen_tex.id);

	m_water.draw(projection_2D);

	//////////////////
	// Presenting
	glfwSwapBuffers(m_window);
}

// Should the game be over ?
bool World::is_over()const
{
	return glfwWindowShouldClose(m_window);
}

// Creates a new enemy and if successfull adds it to the list of enemys
bool World::spawn_enemy_01()
{
	Enemy_01 enemy;
	if (enemy.init(m_points))
	{
		m_enemys_01.emplace_back(enemy);
		return true;
	}
	fprintf(stderr, "Failed to spawn enemy 01");
	return false;
}

// Creates a new enemy and if successfull adds it to the list of enemys
bool World::spawn_enemy_02()
{
	Enemy_02 enemy;
	if (enemy.init(m_points))
	{
		m_enemys_02.emplace_back(enemy);
		return true;
	}
	fprintf(stderr, "Failed to spawn enemy 02");
	return false;
}


// Creates a new enemy and if successfull adds it to the list of enemys
bool World::spawn_enemy_03()
{
	Enemy_03 enemy;
	if (enemy.init(m_points))
	{
		m_enemys_03.emplace_back(enemy);
		return true;
	}
	fprintf(stderr, "Failed to spawn enemy 03");
	return false;
}

// On key callback
void World::on_key(GLFWwindow*, int key, int, int action, int mod)
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// HANDLE SALMON MOVEMENT HERE
	// key is of 'type' GLFW_KEY_
	// action can be GLFW_PRESS GLFW_RELEASE GLFW_REPEAT
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// Resetting game
	int w, h;
        glfwGetFramebufferSize(m_window, &w, &h);
	vec2 screen = { (float)w, (float)h };
	if (action == GLFW_RELEASE && key == GLFW_KEY_R)
	{
		int w, h;
		glfwGetWindowSize(m_window, &w, &h);
		m_hero.destroy();
		start.init(screen);
		m_hero.init(screen);
		m_enemys_01.clear();
		m_enemys_02.clear();
		m_enemys_03.clear();
		hero_projectiles.clear();
		enemy_projectiles.clear();
		m_interface.destroy();
		m_interface.init({ 300.f, 50.f });
		m_water.reset_salmon_dead_time();
		m_current_speed = 1.f;
		screen_left = 0.f;// *-0.5;
		screen_top = 0.f;// (float)h * -0.5;
		screen_right = (float)w;// *0.5;
		screen_bottom = (float)h;// *0.5;
		zoom_factor = 1.f;
		m_points = 0;
		map.set_is_over(true);
		start_is_over = false;
	}

	// Control the current speed with `<` `>`
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) &&  key == GLFW_KEY_COMMA)
		m_current_speed -= 0.1f;
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_PERIOD)
		m_current_speed += 0.1f;

    //add toggle
    if (action == GLFW_RELEASE && key == GLFW_KEY_1) {
        m_hero.advanced = true;
    }
    if (action == GLFW_RELEASE && key == GLFW_KEY_B) {
        m_hero.advanced = false;
    }

	m_current_speed = fmax(0.f, m_current_speed);

	//add key control for direction
	vec2 cur_direction = m_hero.get_direction();

	if (action == GLFW_PRESS && key == GLFW_KEY_D) {
		m_hero.set_direction({1.0f,cur_direction.y});
	}

	else if (action == GLFW_PRESS && key == GLFW_KEY_A) {
		m_hero.set_direction({-1.0f,cur_direction.y});
	}
	else if (action == GLFW_RELEASE && (key == GLFW_KEY_A || key ==GLFW_KEY_D )) {
		m_hero.set_direction({0.0f,cur_direction.y});
	}

	if (action == GLFW_PRESS && key == GLFW_KEY_W) {
		m_hero.set_direction({cur_direction.x,-1.0f});
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_S) {
		m_hero.set_direction({cur_direction.x,1.0f});
	}
	else if (action == GLFW_RELEASE && (key == GLFW_KEY_W || key ==GLFW_KEY_S )) {
		m_hero.set_direction({cur_direction.x,0.0f});
	}
	else if (key == GLFW_KEY_P && start_is_over == true) {
		zoom_factor += 0.1f;
		if ((zoom_factor > 1.5f)) {
			zoom_factor = 1.5f;
		}
	}
	else if (key == GLFW_KEY_O && start_is_over == true) {
		zoom_factor -= 0.1f;
		if ((zoom_factor < 1.1f)) {
			zoom_factor = 1.1f;
		}
	}
	else if (key == GLFW_KEY_G && start_is_over == false) {
		map.init(screen);
		start_is_over = true;
		zoom_factor = 1.1f;
	}
	else if (key == GLFW_KEY_H) {
		//shopping
	} else if (key == GLFW_KEY_ESCAPE) {
		glfwSetWindowShouldClose(m_window, GL_TRUE);
	}
}

void World::on_mouse_move(GLFWwindow* window, double xpos, double ypos)
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// HANDLE SALMON ROTATION HERE
	// xpos and ypos are relative to the top-left of the window, the salmon's
	// default facing direction is (1, 0)
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	if (start_is_over) {
		float angle = 0.f;
		vec2 salmon_position = m_hero.get_position();
		if (xpos - salmon_position.x != 0)
			angle = atan2((ypos - salmon_position.y), (xpos - salmon_position.x));

		m_hero.set_rotation(angle);
	}
}

void World::on_mouse_click(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && start_is_over) {
		shootingFireBall = true;
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE && start_is_over) {
		shootingFireBall = false;
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS && start_is_over)
		m_hero.use_ice_arrow_skill(hero_projectiles);
}

vec2 World::getScreenSize()
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	return { (float)w, (float)h };
}

