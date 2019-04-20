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
	const int INIT_MAX_ENEMIES = 1;
	const int INIT_MAX_ENEMY_03 = 1;
	size_t MAX_ENEMIES_01 = INIT_MAX_ENEMIES;
	size_t MAX_ENEMIES_02 = INIT_MAX_ENEMIES;
	size_t MAX_ENEMIES_03 = INIT_MAX_ENEMY_03;
	const size_t ENEMY_DELAY_MS = 6000;
	const size_t ENEMY_03_DELAY_MS = 10000;
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

	m_window = glfwCreateWindow((int)screen.x, (int)screen.y, "Mystic Crusaders", nullptr, nullptr);

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
	auto mouse_button_callback = [](GLFWwindow* wnd, int _0, int _1, int _2) {((World*)glfwGetWindowUserPointer(wnd))->on_mouse_click(wnd, _0, _1, _2); };
	auto mouse_wheel_callback = [](GLFWwindow* wnd, double _0, double _1) {((World*)glfwGetWindowUserPointer(wnd))->on_mouse_wheel(wnd, _0, _1); };

	glfwSetKeyCallback(m_window, key_redirect);
	glfwSetCursorPosCallback(m_window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(m_window, mouse_button_callback);
	glfwSetScrollCallback(m_window, mouse_wheel_callback);

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
	m_levelup_sound = Mix_LoadWAV(audio_path("level_up.wav"));
	m_lightning_sound = Mix_LoadWAV(audio_path("lightning.wav"));
	m_ice_sound = Mix_LoadWAV(audio_path("ice.wav"));
	m_fireball_sound = Mix_LoadWAV(audio_path("fireball.wav"));
	m_laser_sound = Mix_LoadWAV(audio_path("laser.wav"));
	m_transition_sound = Mix_LoadWAV(audio_path("transition.wav"));
	m_amplify_sound = Mix_LoadWAV(audio_path("amplify.wav"));

	if (m_background_music == nullptr || m_salmon_dead_sound == nullptr
		|| m_salmon_eat_sound == nullptr || m_levelup_sound == nullptr
		|| m_lightning_sound == nullptr || m_ice_sound == nullptr
		|| m_fireball_sound == nullptr || m_laser_sound == nullptr
		|| m_transition_sound == nullptr || m_amplify_sound == nullptr
		)
	{
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
			audio_path("music.wav"),
			audio_path("salmon_dead.wav"),
			audio_path("salmon_eat.wav"),
			audio_path("level_up.wav"),
			audio_path("lightning.wav"),
			audio_path("ice.wav"),
			audio_path("fireball.wav"),
			audio_path("laser.wav"),
			audio_path("transition.wav"),
			audio_path("amplify.wav")
		);
		return false;
	}

	// Playing background music undefinitely
	Mix_PlayMusic(m_background_music, -1);

	fprintf(stderr, "Loaded music\n");

	m_current_speed = 1.f;
	zoom_factor = 1.f;
	start_is_over = start.is_over();
	m_level = 0;
	m_game_level = 0;
	pass_points = 5;
	used_skillpoints = 0;
	skill_num = 0;
	item_num = 0;
	ice_skill_set = { 0.f,0.f,0.f };
	thunder_skill_set = { 0.f,0.f,0.f };
	fire_skill_set = { 0.f,0.f,0.f };
	level_num = { 0.f,0.f,1.f };
	game_is_paused = false;
	shopping = false;
	skill_element = "ice";
	m_window_width = screen.x;
	m_window_height = screen.y;
	stree.init(screen, 1);
	m_hero.init(screen);
	m_portal.init(screen);
	m_skill_switch.init({ 500.f, 500.f });
	passed_level = false;
	shootingFireBall = false;
	cur_points_needed = pass_points - m_points;
	kill_num = number_to_vec(cur_points_needed, true);
	shop.init();
	shop.update_hero(m_hero);

	button_play.makeButton(438, 410, 420, 60, 0.1f, "button_purple.png", "Start", [this]() { this->startGame(); });
	button_play.set_hoverable(true);
	button_tutorial.makeButton(438, 510, 420, 60, 0.1f, "button_purple.png", "Start", [&]() { display_tutorial = true; });
	button_shop.makeButton(438, 610, 420, 60, 0.1f, "button_purple.png", "Start", [&]() { shopping = true; });
	button_back_to_menu.makeButton(801, 30, 429, 90, 0.1f, "button_purple.png", "Start", [&]() { display_tutorial = false; });
	button_back_to_menu2.makeButton(985, 30, 260, 80, 0.1f, "button_purple.png", "Start", [&]() { shopping = false; });
	// For future reference: examples of how to use buttons
	// testButton2.makeButton(500, 600, 200, 50, 0.8f, "button.png", "Start", [&]() { World::startGame(); });
	// testButton2.makeButton(500, 600, 300, 50, 0.8f, "BAR.png", "Tutorial", [this]() { this->doNothing(); });
	// testButton4.makeButton(500, 600, 200, 50, 0.8f, "button.png", "Start", [this]() { this->m_hero.change_mp(80.f); });

	//initialize treetrunk & tree;
	m_treetrunk_position.push_back({ 4* screen.x / 5 - 120.f, screen.y / 3  });
	m_treetrunk_position.push_back({ 4* screen.x / 5 , screen.y / 3 - 50.f });
	m_treetrunk_position.push_back({ screen.x / 4 , screen.y / 4 });
	m_treetrunk_position.push_back({ screen.x / 4 - 120.f , screen.y / 4 + 50.f });
	m_treetrunk_position.push_back({ 2* screen.x  / 3 , screen.y *3/ 4  });


	initTrees();

	mouse_position = { 0.f,0.f };
	return start.init(screen) && m_water.init() && m_interface.init({ 300.f, 42.f }) && m_tutorial.init(screen) && shop_screen.init(screen) && hme.init(screen) && ingame.init(screen);
}

bool World::initTrees() {
	if (m_game_level % 2 == 0) {
		for (auto & position : m_treetrunk_position)
		{
			if (!spawn_treetrunk())
				return false;

			Treetrunk& new_trunk = m_treetrunk.back();
			new_trunk.set_position({ position.x,position.y + 200.f });
		}

		for (auto & position : m_treetrunk_position)
		{
			if (!spawn_tree())
				return false;

			Tree& new_tree = m_tree.back();
			new_tree.set_position({ position.x + 10.f ,position.y });
		}
	}
	else
	{
		for (auto & position : m_treetrunk_position)
		{
			if (!spawn_vine())
				return false;

			Vine& new_vine = m_vine.back();
			new_vine.set_position({ position.x,position.y - 30.f });
		}
	}

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
	if (m_ice_sound != nullptr)
		Mix_FreeChunk(m_ice_sound);
	if (m_lightning_sound != nullptr)
		Mix_FreeChunk(m_lightning_sound);
	if (m_fireball_sound != nullptr)
		Mix_FreeChunk(m_fireball_sound);
	if (m_laser_sound != nullptr)
		Mix_FreeChunk(m_laser_sound);
	if (m_transition_sound != nullptr)
		Mix_FreeChunk(m_transition_sound);
	if (m_amplify_sound != nullptr)
		Mix_FreeChunk(m_amplify_sound);

	Mix_CloseAudio();

	m_hero.destroy();
	for (auto& enemy : m_enemys_01)
		enemy.destroy(true);
	for (auto& enemy : m_enemys_02)
		enemy.destroy(true);
	for (auto& enemy : m_enemys_03)
		enemy.destroy(true);
	for (auto& h_proj : hero_projectiles)
		h_proj->destroy();
	for (auto& e_proj : enemy_projectiles)
		e_proj.destroy();
	for (auto& tree : m_tree)
		tree.destroy();
	for (auto& treetrunk : m_tree)
		treetrunk.destroy();
	for (auto& vine : m_vine)
		vine.destroy();
	for (auto& thunder : thunders)
		thunder->destroy();
	for (auto& phoenix : phoenix_list)
		phoenix->destroy(true);

	m_enemys_01.clear();
	m_enemys_02.clear();
	hero_projectiles.clear();
	enemy_projectiles.clear();
	m_interface.destroy();
	ingame.destroy();
	m_skill_switch.destroy();
	start.destroy();
	stree.destroy();
	hme.destroy();
	button_play.destroy();
	button_tutorial.destroy();
	button_shop.destroy();
	button_back_to_menu.destroy();
	button_back_to_menu2.destroy();
	glfwDestroyWindow(m_window);
}

// Update our game world
bool World::update(float elapsed_ms)
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	vec2 screen = { (float)w, (float)h };

	start.update(start_is_over);
	stree.update_skill(game_is_paused, m_level, used_skillpoints,ice_skill_set, thunder_skill_set, fire_skill_set, skill_num, screen);
	current_stock = shop.get_stock(find_item(item_num));
	current_price = shop.get_price(find_item(item_num));
	balance = shop.get_balance();
	shop_screen.update_shop(shopping, current_stock, balance - current_price, item_num, screen);

	if (passed_level && m_hero.justFinishedTransition) {
		map.destroy();
		passed_level = !passed_level;
		m_hero.justFinishedTransition = false;
		m_portal.setIsPortal(false);
		m_game_level++;
		m_hero.hp = m_hero.max_hp;
		m_hero.mp = m_hero.max_mp;
		for (auto& treetrunk : m_treetrunk)
			treetrunk.destroy();
		for (auto& tree : m_tree)
			tree.destroy();
		for (auto& vine : m_vine)
			vine.destroy();
		m_tree.clear();
		m_treetrunk.clear();
		m_vine.clear();
		initTrees();
		map.init(screen, m_game_level);
		pass_points = m_points + (m_game_level + 1) * 5;
		cur_points_needed = pass_points - m_points;
		//kill_num = number_to_vec(cur_points_needed, true);
	}

	if (start_is_over && !game_is_paused && !shopping && !m_hero.isInTransition) {
		if (m_hero.is_alive()) {

			if (shootingFireBall && clock() - lastFireProjectileTime > 300) {
				m_hero.shoot_projectiles(hero_projectiles);
				Mix_PlayChannel(-1, m_fireball_sound, 0);
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



			auto vine = m_vine.begin();
			while (vine != m_vine.end())
			{
				if (m_hero.collides_with(*vine))
				{
					m_hero.take_damage(vine->get_damage());

					if (!m_hero.is_alive()) {
						Mix_PlayChannel(-1, m_salmon_dead_sound, 0);
						m_water.set_salmon_dead();
						m_hero.kill();
					}
					break;
				}
				++vine;
			}

			// Checking hero - Enemy collisions
			auto e_proj = enemy_projectiles.begin();
			while (e_proj != enemy_projectiles.end())
			{
				if (m_hero.collides_with(*e_proj))
				{
					m_hero.take_damage(e_proj->get_damage());
					//comment back later
					e_proj->destroy();
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
				vec2 force = { 0.f, -10.f };
				m_hero.apply_momentum(force);
			}
			else if (m_hero.get_position().y < m_hero.m_scale.y * 2) {
				vec2 force = { 0.f, 10.f };
				m_hero.apply_momentum(force);
			}

			if (m_hero.get_position().x > m_window_width - m_hero.m_scale.x * 2) {
				vec2 force = { -10.f, 0.f };
				m_hero.apply_momentum(force);
			}
			else if (m_hero.get_position().x < m_hero.m_scale.x * 2) {
				vec2 force = { 10.f, 0.f };

				m_hero.apply_momentum(force);
			}
			if (m_points * m_hero.exp_multiplier - previous_point > 15 + (m_hero.level * 5))
			{
				previous_point = m_points;
				m_hero.levelup();
				m_level++;
				Mix_PlayChannel(-1, m_levelup_sound, 0);
			}
			if (m_points >= pass_points && !passed_level) {
				m_portal.setIsPortal(true);
				passed_level = true;
				m_portal.killAll(thunders);
				Mix_PlayChannel(-1, m_transition_sound, 0);
			}
		}

		for (Enemy_01 enemy : m_enemys_01)
		{
			if (enemy.needFireProjectile == true)
			{
				enemy.shoot_projectiles(enemy_projectiles);
				Mix_PlayChannel(-1, m_laser_sound, 0);
			}
		}

		for (auto & enemy : m_enemys_03)
		{
			if (enemy.needFireProjectile == true)
			{
				enemy.set_wave();
				Mix_PlayChannel(-1, m_amplify_sound, 0);
				int rand_factor = 0 + (std::rand() % (1 - 0 + 1));
				if (rand_factor == 0)
				{
					if (m_enemys_01.size() > 0) {
						int factor = m_enemys_01.size();
						if (factor == 0) {
							enemy.recentPowerupType = m_enemys_01[0].powerup();
							m_enemys_01[0].set_wave();
						}
						else {
							rand_factor = std::rand() % factor + 0;
							enemy.recentPowerupType = m_enemys_01[rand_factor].powerup();
							m_enemys_01[rand_factor].set_wave();
						}
					}
				}
				else {
					if (m_enemys_02.size() > 0) {
						int factor = m_enemys_02.size();
						if (factor == 0) {
							enemy.recentPowerupType = m_enemys_02[0].powerup();
							m_enemys_02[0].set_wave();
						}
						else {
							rand_factor = std::rand() % factor + 0;
							enemy.recentPowerupType = m_enemys_02[rand_factor].powerup();
							m_enemys_02[rand_factor].set_wave();
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
		for (auto& vine : m_vine)
			vine.update(elapsed_ms * m_current_speed);
		m_portal.update(elapsed_ms * m_current_speed, cur_points_needed - (pass_points - m_points), cur_points_needed);
		m_interface.update({ m_hero.get_hp(), m_hero.get_mp() }, {(float) (m_points - previous_point), (float) (20 + (m_hero.level * 5))}, zoom_factor);
		for (auto& thunder : thunders)
			thunder->update(elapsed_ms);
		for (auto& phoenix : phoenix_list)
			phoenix->update(elapsed_ms,m_hero.get_position(),m_enemys_01, m_enemys_02, m_enemys_03,hero_projectiles);
		m_interface.update({ m_hero.get_hp(), m_hero.get_mp() }, { (float)(m_points - previous_point), (float)(20 + (m_hero.level * 5)) }, zoom_factor);
		hme.update_hme(m_hero.get_position(), zoom_factor, screen);
		level_num = number_to_vec(m_game_level, false);
		kill_num = number_to_vec(pass_points - m_points, true);
		ingame.update_ingame(start_is_over, level_num, kill_num, screen, m_hero.get_position(), zoom_factor);
		m_skill_switch.update(m_hero.get_active_skill(), zoom_factor);
		//check portal collision
		for (auto &e1 : m_enemys_01)
		{
			if (m_portal.collides_with(e1))
			{
				vec2 cur_position = e1.get_position();
				int facing = e1.m_face_left_or_right;
				facing = (facing == 0) ? facing - 1 : facing;
				facing = facing * -10;
				vec2 new_position = { cur_position.x + facing, cur_position.y + 5 };
				e1.set_position(new_position);
			}
		}

		for (auto &e2 : m_enemys_02)
		{
			if (m_portal.collides_with(e2))
			{
				vec2 cur_position = e2.get_position();
				int facing = e2.m_face_left_or_right;
				facing = (facing == 0) ? facing - 1 : facing;
				facing = facing * -10;
				vec2 new_position = { cur_position.x + facing, cur_position.y + 5 };
				e2.set_position(new_position);
			}
		}

		for (auto &e3 : m_enemys_03)
		{
			if (m_portal.collides_with(e3))
			{
				vec2 cur_position = e3.get_position();
				int facing = e3.m_face_left_or_right;
				facing = (facing == 0) ? facing - 1 : facing;
				facing = facing * -10;
				vec2 new_position = { cur_position.x + facing, cur_position.y + 5 };
				e3.set_position(new_position);
			}
		}

		if (m_portal.collides_with(m_hero)) {
			vec2 cur_direction = m_hero.get_direction();
			vec2 cur_position = m_hero.get_position();
			float stepback = elapsed_ms * -0.6; // -0.2 is 200 / 1000, which is in hero.cpp so 0.6 to stepback more so the hero does not stuck on it
			vec2 new_position = { cur_position.x + cur_direction.x * stepback , cur_position.y + cur_direction.y * stepback };
			m_hero.set_position(new_position);
			if(passed_level && !m_hero.isInTransition) {
				m_hero.next_level();
			}
		}

		int p_len = (int)hero_projectiles.size() - 1;
		for (int i = p_len; i >= 0; i--)
		{
			Projectile* h_proj = hero_projectiles.at(i);
			if (m_portal.collides_with(*h_proj))
			{
				h_proj->destroy();
				hero_projectiles.erase(hero_projectiles.begin() + i);
			}
		}

		//same for enemy projectile
		int l_len = (int)enemy_projectiles.size() - 1;
		for (int i = l_len; i >= 0; i--)
		{
			EnemyLaser laser = enemy_projectiles.at(i);
			if (m_portal.collides_with(laser))
			{
				laser.destroy();
				enemy_projectiles.erase(enemy_projectiles.begin() + i);
			}
		}



		//check treetrunk collision
		//some bugs in collision detection need to be fixed latter, but it is not related to here
		for (auto &treeTrunk : m_treetrunk)
		{

			if (treeTrunk.collide_with(m_hero)) {
				vec2 cur_position = m_hero.get_position();
				vec2 tree_location = treeTrunk.get_position();
				vec2 current_direction = m_hero.get_direction();

				//find the difference vector, but only push back hero in the opposite direction that the hero walks
				vec2 difference = { (cur_position.x - tree_location.x)* abs(current_direction.x), (cur_position.y - tree_location.y) * abs(current_direction.y) };
				difference = { difference.x + 0.001f, difference.y + 0.001f }; //add 0.0001f to avoid divide by 0
				float size = sqrtf(dot(difference, difference));
				difference = { difference.x / size, difference.y / size }; //scale the difference
				float stepback = elapsed_ms * 0.6; // -0.2 is 200 / 1000, which is in hero.cpp so 0.4 to stepback more so the hero does not stuck on it
				vec2 new_position = { cur_position.x + difference.x * stepback, cur_position.y + difference.y * stepback  };

				m_hero.set_position(new_position);
			}

			// if hero projectile hit the tree trunck then destroy it
			int p_len = (int)hero_projectiles.size() - 1;
			for (int i = p_len; i >= 0; i--)
			{
				Projectile* h_proj = hero_projectiles.at(i);
				if (treeTrunk.collide_with(*h_proj))
				{
					h_proj->destroy();
					hero_projectiles.erase(hero_projectiles.begin() + i);
				}
			}

			//same for enemy projectile
			l_len = (int)enemy_projectiles.size() - 1;
			for (int i = l_len; i >= 0; i--)
			{
				EnemyLaser laser = enemy_projectiles.at(i);
				if (treeTrunk.collide_with(laser))
				{
					laser.destroy();
					enemy_projectiles.erase(enemy_projectiles.begin() + i);
				}
			}

			for (auto &e1 : m_enemys_01)
			{
				if (treeTrunk.collide_with(e1))
				{
					vec2 cur_position = e1.get_position();
					vec2 tree_location = treeTrunk.get_position();
					vec2 difference = { cur_position.x - tree_location.x, cur_position.y - tree_location.y };
					float size = sqrtf(dot(difference, difference));
					difference = { difference.x / size, difference.y / size };
					float stepback = elapsed_ms * 0.5;
					vec2 new_position = { cur_position.x + difference.x * stepback, cur_position.y + difference.y * stepback };
					e1.set_position(new_position);
				}
			}

			for (auto &e2 : m_enemys_02)
			{
				if (treeTrunk.collide_with(e2))
				{
					vec2 cur_position = e2.get_position();
					vec2 tree_location = treeTrunk.get_position();
					vec2 difference = { cur_position.x - tree_location.x, cur_position.y - tree_location.y };
					float size = sqrtf(dot(difference, difference));
					difference = { difference.x / size, difference.y / size };
					float stepback = elapsed_ms * 0.5;
					vec2 new_position = { cur_position.x + difference.x * stepback, cur_position.y + difference.y * stepback };
					e2.set_position(new_position);
				}
			}

			for (auto &e3 : m_enemys_03)
			{
				if (treeTrunk.collide_with(e3))
				{
					vec2 cur_position = e3.get_position();
					vec2 tree_location = treeTrunk.get_position();
					vec2 difference = { cur_position.x - tree_location.x, cur_position.y - tree_location.y };
					float size = sqrtf(dot(difference, difference));
					difference = { difference.x / size, difference.y / size };
					float stepback = elapsed_ms * 0.5;
					vec2 new_position = { cur_position.x + difference.x * stepback, cur_position.y + difference.y * stepback };
					e3.set_position(new_position);
				}
			}
		}

		//check collision between phoenix and enemies

		//remove out of screen fireball

		int len = (int)hero_projectiles.size() - 1;
		for (int i = len; i >= 0; i--)
		{
			Projectile* h_proj = hero_projectiles.at(i);
			if (h_proj->get_position().x < 0.f || h_proj->get_position().x > screen.x || h_proj->get_position().y < 0.f || h_proj->get_position().y > screen.y)
			{
				h_proj->destroy();
				hero_projectiles.erase(hero_projectiles.begin() + i);
				continue;
			}
		}

		//remove out of screen lasers
		auto e_proj = enemy_projectiles.begin();
		while (e_proj != enemy_projectiles.end())
		{
			if (e_proj->get_position().x < 0.f || e_proj->get_position().x > screen.x || e_proj->get_position().y < 0.f || e_proj->get_position().y > screen.y)
			{
				e_proj->destroy();
				e_proj = enemy_projectiles.erase(e_proj);
				continue;
			}

			++e_proj;
		}

		//remove overtime thunder
		len = (int)thunders.size() - 1;
		for (int i = len; i >= 0; i--)
		{
			Thunder* t = thunders.at(i);

			if (t->can_remove())
			{
				t->destroy();
				thunders.erase(thunders.begin() + i);
				continue;
			}
		}

		//remove overtime phoenix
		len = (int)phoenix_list.size() - 1;
		for (int i = len; i >= 0; i--)
		{
			phoenix* p = phoenix_list.at(i);

			if (!p->is_alive())
			{
				p->destroy(true);
				phoenix_list.erase(phoenix_list.begin() + i);
				continue;
			}
		}

		auto enemy = m_enemys_01.begin();

		while (enemy != m_enemys_01.end())
		{
			int len = (int)hero_projectiles.size() - 1;
			for (int i = len; i >= 0; i--)
			{
				Projectile* h_proj = hero_projectiles.at(i);
				if (enemy->collide_with(*h_proj))
				{
					enemy->take_damage(h_proj->get_damage(), h_proj->get_velocity());
					h_proj->destroy();
					hero_projectiles.erase(hero_projectiles.begin() + i);
					if (!enemy->is_alive()) {
						enemy->destroy(true);
						enemy = m_enemys_01.erase(enemy);
						if (!passed_level){
							++m_points;
						}
						MAX_ENEMIES_01 = INIT_MAX_ENEMIES + (m_points / 23);
					}
					break;
				}

			}

			if (enemy == m_enemys_01.end() || m_enemys_01.size() == 0) {
				break;
			}
			++enemy;
		}

		enemy = m_enemys_01.begin();

		while (enemy != m_enemys_01.end())
		{
			int len = (int)thunders.size() - 1;
			for (int i = len; i >= 0; i--)
			{
				Thunder* t = thunders.at(i);
				if (enemy->collide_with(*t))
				{
					t->apply_effect(*enemy);

					if (!enemy->is_alive()) {
						enemy->destroy(true);
						enemy = m_enemys_01.erase(enemy);
						if (!passed_level){
							++m_points;
						}
						MAX_ENEMIES_01 = INIT_MAX_ENEMIES + m_points / 23;
					}
					break;
				}

			}

			if (enemy == m_enemys_01.end() || m_enemys_01.size() == 0) {
				break;
			}
			++enemy;
		}

		auto enemy2 = m_enemys_02.begin();

		while (enemy2 != m_enemys_02.end())
		{
			int len = (int)hero_projectiles.size() - 1;
			for (int i = len; i >= 0; i--)
			{
				Projectile* h_proj = hero_projectiles.at(i);
				if (enemy2->collide_with(*h_proj))
				{
					enemy2->take_damage(h_proj->get_damage(), h_proj->get_velocity());
					h_proj->destroy();
					hero_projectiles.erase(hero_projectiles.begin() + i);
					if (!enemy2->is_alive()) {
						enemy2->destroy(true);
						enemy2 = m_enemys_02.erase(enemy2);
						if (!passed_level){
							++m_points;
						}
						MAX_ENEMIES_02 = INIT_MAX_ENEMIES + (m_points / 17);
					}
					break;
				}
			}
			if (enemy2 == m_enemys_02.end() || m_enemys_02.size() == 0) {
				break;
			}
			++enemy2;
		}

		enemy2 = m_enemys_02.begin();

		while (enemy2 != m_enemys_02.end())
		{
			int len = (int)thunders.size() - 1;
			for (int i = len; i >= 0; i--)
			{
				Thunder* t = thunders.at(i);
				if (enemy2->collide_with(*t))
				{
					t->apply_effect(*enemy2);

					if (!enemy2->is_alive()) {
						enemy2->destroy(true);
						enemy2 = m_enemys_02.erase(enemy2);
						if (!passed_level){
							++m_points;
						}
						MAX_ENEMIES_02 = INIT_MAX_ENEMIES + m_points / 17;
					}
					break;
				}

			}

			if (enemy2 == m_enemys_02.end() || m_enemys_02.size() == 0) {
				break;
			}
			++enemy2;
		}

		auto enemy3 = m_enemys_03.begin();

		while (enemy3 != m_enemys_03.end())
		{
			int len = (int)hero_projectiles.size() - 1;
			for (int i = len; i >= 0; i--)
			{
				Projectile* h_proj = hero_projectiles.at(i);
				if (enemy3->collide_with(*h_proj))
				{
					enemy3->take_damage(h_proj->get_damage(), h_proj->get_velocity());
					h_proj->destroy();
					hero_projectiles.erase(hero_projectiles.begin() + i);
					if (!enemy3->is_alive()) {
						enemy3->destroy(true);
						enemy3 = m_enemys_03.erase(enemy3);
						if (!passed_level){
							++m_points;
						}
						MAX_ENEMIES_03 = INIT_MAX_ENEMY_03 + (m_points / 41);
					}
					break;
				}
			}
			if (enemy3 == m_enemys_03.end() || m_enemys_03.size() == 0) {
				break;
			}
			++enemy3;
		}

		enemy3 = m_enemys_03.begin();

		while (enemy3 != m_enemys_03.end())
		{
			int len = (int)thunders.size() - 1;
			for (int i = len; i >= 0; i--)
			{
				Thunder* t = thunders.at(i);
				if (enemy3->collide_with(*t))
				{
					t->apply_effect(*enemy3);

					if (!enemy3->is_alive()) {
						enemy3->destroy(true);
						enemy3 = m_enemys_03.erase(enemy3);
						if (!passed_level){
							++m_points;
						}
						MAX_ENEMIES_03 = INIT_MAX_ENEMIES + m_points / 41;
					}
					break;
				}

			}

			if (enemy3 == m_enemys_03.end() || m_enemys_03.size() == 0) {
				break;
			}
			++enemy3;
		}

		//check collision with phoenix

		enemy = m_enemys_01.begin();

		while (enemy != m_enemys_01.end())
		{
			int len = (int)phoenix_list.size() - 1;
			for (int i = len; i >= 0; i--)
			{
				phoenix* p = phoenix_list.at(i);
				if (p->collide_with(*enemy))
				{
					p->change_hp(-1.f);
					enemy->take_damage(6.f);

					if (!enemy->is_alive()) {
						enemy->destroy(true);
						enemy = m_enemys_01.erase(enemy);
						++m_points;
						MAX_ENEMIES_01 = INIT_MAX_ENEMIES + m_points / 23;
					}
					break;
				}

			}

			if (enemy == m_enemys_01.end() || m_enemys_01.size() == 0) {
				break;
			}
			++enemy;
		}

		enemy2 = m_enemys_02.begin();

		while (enemy2 != m_enemys_02.end())
		{
			int len = (int)phoenix_list.size() - 1;
			for (int i = len; i >= 0; i--)
			{
				phoenix* p = phoenix_list.at(i);
				if (p->collide_with(*enemy2))
				{
					p->change_hp(-1.f);
					enemy2->take_damage(6.f);

					if (!enemy2->is_alive()) {
						enemy2->destroy(true);
						enemy2 = m_enemys_02.erase(enemy2);
						++m_points;
						MAX_ENEMIES_02 = INIT_MAX_ENEMIES + m_points / 17;
					}
					break;
				}

			}

			if (enemy2 == m_enemys_02.end() || m_enemys_02.size() == 0) {
				break;
			}
			++enemy2;
		}

		enemy3 = m_enemys_03.begin();

		while (enemy3 != m_enemys_03.end())
		{
			int len = (int)phoenix_list.size() - 1;
			for (int i = len; i >= 0; i--)
			{
				phoenix* p = phoenix_list.at(i);
				if (p->collide_with(*enemy3))
				{
					p->change_hp(-1.f);
					enemy3->take_damage(6.f);

					if (!enemy3->is_alive()) {
						enemy3->destroy(true);
						enemy3 = m_enemys_03.erase(enemy3);
						++m_points;
						MAX_ENEMIES_03 = INIT_MAX_ENEMIES + m_points / 17;
					}
					break;
				}

			}

			if (enemy3 == m_enemys_03.end() || m_enemys_03.size() == 0) {
				break;
			}
			++enemy3;
		}

		//check collision between phoenix and enemy laser
		l_len = (int)enemy_projectiles.size() - 1;
		for (int i = l_len; i >= 0; i--)
		{
			EnemyLaser &laser = enemy_projectiles.at(i);
			int j_len = (int)phoenix_list.size() - 1;
			for (int j = j_len; j >= 0; j--)
			{
				phoenix* p = phoenix_list.at(j);
				if (p->collide_with(laser))
				{
					p->change_hp(-10.f);
					laser.destroy();
					enemy_projectiles.erase(enemy_projectiles.begin() + i);
					break;
				}
			}

		}



		// Spawning new enemys
		if (!passed_level){
			m_next_enemy1_spawn -= elapsed_ms * m_current_speed;
			if (m_enemys_01.size() < MAX_ENEMIES_01 && m_next_enemy1_spawn < 0.f && m_points >= 3)
			{
				if (!spawn_enemy_01())
					return false;

				Enemy_01& new_enemy = m_enemys_01.back();

				int left_or_right_spawn = rand() % 2;

				float screen_x = 0;

				if (left_or_right_spawn == 0) {
					screen_x = screen.x + 150.f;
				}

				// Setting random initial position
				new_enemy.set_position({ screen_x, 50 + m_dist(m_rng) * (screen.y - 100) });

				// Next spawn
				m_next_enemy1_spawn = m_dist(m_rng) * (ENEMY_DELAY_MS)-log(m_points + 1) * 300;
			}
			m_next_enemy2_spawn -= elapsed_ms * m_current_speed;
			if (m_enemys_02.size() < MAX_ENEMIES_02 && m_next_enemy2_spawn < 0.f)
			{
				if (!spawn_enemy_02())
					return false;

				Enemy_02& new_enemy = m_enemys_02.back();

				int left_or_right_spawn = rand() % 2;

				float screen_x = 0;

				if (left_or_right_spawn == 0) {
					screen_x = screen.x + 150.f;
				}

				// Setting random initial position
				new_enemy.set_position({ screen_x, 50 + m_dist(m_rng) * (screen.y - 100) });

				// Next spawn
				m_next_enemy2_spawn = m_dist(m_rng) * (ENEMY_DELAY_MS)-log(m_points + 1) * 300;
			}

			m_next_enemy3_spawn -= elapsed_ms * m_current_speed;
			if (m_enemys_03.size() < MAX_ENEMIES_03 && m_next_enemy3_spawn < 0.f && m_points >= 15)
			{
				if (!spawn_enemy_03())
					return false;

				Enemy_03& new_enemy = m_enemys_03.back();

				int left_or_right_spawn = rand() % 2;

				float screen_x = 0;

				if (left_or_right_spawn == 0) {
					screen_x = screen.x + 150.f;
				}

				// Setting random initial position
				new_enemy.set_position({ screen_x, 50 + m_dist(m_rng) * (screen.y - 100) });

				// Next spawn
				m_next_enemy3_spawn = m_dist(m_rng) * (ENEMY_03_DELAY_MS)-log(m_points + 1) * 300;
			}
		}
	}

	// If hero is dead, restart the game after the fading animation
	if (!m_hero.is_alive() &&
		m_water.get_salmon_dead_time() > 5) {
		int w, h;
		glfwGetWindowSize(m_window, &w, &h);
		stree.destroy();
		start.destroy();
		m_interface.destroy();
		hme.destroy();
		ingame.destroy();
		button_play.destroy();
		button_tutorial.destroy();
		button_shop.destroy();
		button_back_to_menu.destroy();
		button_back_to_menu2.destroy();
		m_tutorial.destroy();
		shop_screen.destroy();
		m_hero.destroy();
		for (auto& enemy : m_enemys_01)
			enemy.destroy(true);
		for (auto& enemy : m_enemys_02)
			enemy.destroy(true);
		for (auto& enemy : m_enemys_03)
			enemy.destroy(true);
		for (auto& h_proj : hero_projectiles)
			h_proj->destroy();
		for (auto& e_proj : enemy_projectiles)
			e_proj.destroy();
		for (auto& tree : m_tree)
			tree.destroy();
		for (auto& treetrunk : m_treetrunk)
			treetrunk.destroy();
		for (auto& thunder : thunders)
			thunder->destroy();
		for (auto& phoenix : phoenix_list)
			phoenix->destroy(true);
		for (auto& vine : m_vine)
			vine.destroy();
		m_hero.init(screen);
		shop.update_hero(m_hero);
		button_play.makeButton(438, 410, 420, 60, 0.1f, "button_purple.png", "Start", [this]() { this->startGame(); });
		button_tutorial.makeButton(438, 510, 420, 60, 0.1f, "button_purple.png", "Start", [&]() { display_tutorial = true; });
		button_shop.makeButton(438, 610, 420, 60, 0.1f, "button_purple.png", "Start", [&]() { shopping = true; });
		button_back_to_menu.makeButton(801, 30, 429, 90, 0.1f, "button_purple.png", "Start", [&]() { display_tutorial = false; });
		button_back_to_menu2.makeButton(985, 30, 260, 80, 0.1f, "button_purple.png", "Start", [&]() { shopping = false; });
		m_tutorial.init(screen);
		shop_screen.init(screen);
		start.init(screen);
		m_enemys_01.clear();
		m_enemys_02.clear();
		m_enemys_03.clear();
		hero_projectiles.clear();
		enemy_projectiles.clear();
		thunders.clear();
		phoenix_list.clear();
		m_skill_switch.destroy();
		m_interface.destroy();
		m_interface.init({ 300.f, 50.f });
		m_treetrunk.clear();
		m_tree.clear();
		m_vine.clear();
		m_skill_switch.init({ 500.f, 500.f });
		m_water.reset_salmon_dead_time();
		m_current_speed = 1.f;
		zoom_factor = 1.f;
		shop.set_balance(shop.get_balance() + m_points);
		m_points = 0;
		m_portal.setIsPortal(false);
		passed_level = false;
		m_level = 0;
		m_game_level = 0;
		initTrees();
		used_skillpoints = 0;
		skill_num = 0;
		item_num = 0;
		ice_skill_set = { 0.f,0.f,0.f };
		fire_skill_set = { 0.f,0.f,0.f };
		thunder_skill_set = { 0.f,0.f,0.f };
		fire_skill_set = { 0.f,0.f,0.f };
		level_num = { 0.f,0.f,1.f };
		game_is_paused = false;
		shopping = false;
		previous_point = 0;
		pass_points = 5;
		map.set_is_over(true);
		start_is_over = false;
		display_tutorial = false;
		start.init(screen);
		stree.init(screen, 1);
		m_interface.init({ 300.f, 42.f });
		ingame.init(screen);
		hme.init(screen);
		cur_points_needed = pass_points - m_points;
		kill_num = number_to_vec(cur_points_needed, true);
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
	title_ss << "Points: " << m_points << " HP:" << m_hero.get_hp() << "MP:" <<m_hero.get_mp() << "Level: " << m_game_level ;
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

	float sx = zoom_factor * 2.f / (screen_right - screen_left);
	float sy = zoom_factor * 2.f / (screen_top - screen_bottom);

	vec2 salmon_position = m_hero.get_position();
	our_x = salmon_position.x;
	our_y = salmon_position.y;

	float w_not_scaled = (float)w;
	float h_not_scaled = (float)h;
	float w_scaled = (float)w * zoom_factor;
	float h_scaled = (float)h * zoom_factor;
	screen_left = our_x * zoom_factor - (w_not_scaled / 2);

	if (screen_left < m_hero.m_scale.x * 2) {
		screen_left = m_hero.m_scale.x * 2;
	}
	else if (screen_left + w_not_scaled > w_scaled - m_hero.m_scale.x * 2) {
		screen_left = w_scaled - w_not_scaled - m_hero.m_scale.x * 2;
	}
	screen_top = our_y * zoom_factor - (h_not_scaled / 2);
	if (screen_top < m_hero.m_scale.y * 2 * zoom_factor) {
		screen_top = m_hero.m_scale.y * 2 * zoom_factor;
	}
	else if (screen_top + h_not_scaled > h_scaled - m_hero.m_scale.y * 2) {
		screen_top = h_scaled - h_not_scaled - m_hero.m_scale.y * 2;
	}
	screen_right = screen_left + w_not_scaled;
	screen_bottom = screen_top + h_not_scaled;

	// for our UI bar
	m_interface.set_position({ screen_left, screen_top }, h, 15);
	hme.set_position({ screen_left, screen_top }, h, 12);
	ingame.set_position({ screen_left, screen_top }, h + 15, w-200);
	m_skill_switch.set_position({ screen_left + 400.f, screen_top + 550.f });
	float tx = -1 * (screen_right + screen_left) / (screen_right - screen_left);
	float ty = -1 * (screen_top + screen_bottom) / (screen_top - screen_bottom);

	mat3 scaling_2D{ { sx, 0.f, 0.f },{ 0.f, sy, 0.f },{ 0.f, 0.f, 1.f} };
	mat3 translate_2D{ { 1.f, 0.f, 0.f },{ 0.f, 1.f, 0.f },{ tx, ty, 1.f} };

	mat3 projection_2D = mul(translate_2D, scaling_2D);

	start.draw(projection_2D);
	if (!display_tutorial && !shopping) {
		button_play.draw(projection_2D);
		button_tutorial.draw(projection_2D);
		button_shop.draw(projection_2D);
	}

	if (display_tutorial) {
		m_tutorial.draw(projection_2D);
		button_back_to_menu.draw(projection_2D);
	}
	if (shopping) {
		shop_screen.draw(projection_2D);
		button_back_to_menu2.draw(projection_2D);
	}

	// Drawing entities


	if (start_is_over && !shopping) {
		map.draw(projection_2D);
		m_hero.draw(projection_2D);
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
		for (auto& thunder : thunders)
			thunder->draw(projection_2D);
		for (auto& phoenix : phoenix_list)
			phoenix->draw(projection_2D);

		for (auto& treetrunk : m_treetrunk)
			treetrunk.draw(projection_2D);
		for (auto& tree : m_tree)
			tree.draw(projection_2D);
		for (auto& vine : m_vine)
			vine.draw(projection_2D);
		m_interface.draw(projection_2D);
		hme.draw(projection_2D);
		ingame.draw(projection_2D);
		m_skill_switch.draw(projection_2D);
		m_portal.draw(projection_2D);
	}

	if (game_is_paused){
		stree.draw(projection_2D);
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


bool World::spawn_treetrunk()
{
	Treetrunk treetrunk;
	if (treetrunk.init({ m_window_width,m_window_height }))
	{
		m_treetrunk.emplace_back(treetrunk);
		return true;
	}
	fprintf(stderr, "Failed to spawn treetrunk");
	return false;
}


bool World::spawn_tree()
{
	Tree tree;
	if (tree.init({ m_window_width,m_window_height }))
	{
		m_tree.emplace_back(tree);
		return true;
	}
	fprintf(stderr, "Failed to spawn treetrunk");
	return false;
}

bool World::spawn_vine()
{
	Vine vine;
	if (vine.init({ m_window_width,m_window_height }))
	{
		m_vine.emplace_back(vine);
		return true;
	}
	fprintf(stderr, "Failed to spawn vine");
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
	if (action == GLFW_RELEASE && key == GLFW_KEY_R && start_is_over == true && shopping == false)
	{
		int w, h;
		glfwGetWindowSize(m_window, &w, &h);
		button_play.destroy();
		button_tutorial.destroy();
		button_shop.destroy();
		button_back_to_menu.destroy();
		button_back_to_menu2.destroy();
		m_tutorial.destroy();
		shop_screen.destroy();
		hme.destroy();
		map.destroy();
		m_hero.destroy();
		start.destroy();
		stree.destroy();
		m_interface.destroy();
		ingame.destroy();
		m_skill_switch.destroy();
		for (auto& enemy : m_enemys_01)
			enemy.destroy(true);
		for (auto& enemy : m_enemys_02)
			enemy.destroy(true);
		for (auto& enemy : m_enemys_03)
			enemy.destroy(true);
		for (auto& h_proj : hero_projectiles)
			h_proj->destroy();
		for (auto& e_proj : enemy_projectiles)
			e_proj.destroy();
		for (auto& tree : m_tree)
			tree.destroy();
		for (auto& treetrunk : m_treetrunk)
			treetrunk.destroy();
		for (auto& thunder : thunders)
			thunder->destroy();
		for (auto& phoenix : phoenix_list)
			phoenix->destroy(true);
		for (auto& vine : m_vine)
			vine.destroy();
		m_treetrunk.clear();
		m_tree.clear();
		m_vine.clear();
		start.init(screen);
		button_play.makeButton(438, 410, 420, 60, 0.1f, "button_purple.png", "Start", [this]() { this->startGame(); });
		button_tutorial.makeButton(438, 510, 420, 60, 0.1f, "button_purple.png", "Start", [&]() { display_tutorial = true; });
		button_shop.makeButton(438, 610, 420, 60, 0.1f, "button_purple.png", "Start", [&]() { shopping = true; });
		button_back_to_menu.makeButton(801, 30, 429, 90, 0.1f, "button_purple.png", "Start", [&]() { display_tutorial = false; });
		button_back_to_menu2.makeButton(985, 30, 260, 80, 0.1f, "button_purple.png", "Start", [&]() { shopping = false; });
		m_tutorial.init(screen);
		shop_screen.init(screen);
		m_hero.init(screen);
		m_enemys_01.clear();
		m_enemys_02.clear();
		m_enemys_03.clear();
		hero_projectiles.clear();
		enemy_projectiles.clear();
		thunders.clear();
		m_interface.init({ 300.f, 42.f });
		stree.init(screen, 1);
		ingame.init(screen);
		hme.init(screen);
		phoenix_list.clear();
		m_skill_switch.init({ 500.f, 500.f });
		m_water.reset_salmon_dead_time();
		m_current_speed = 1.f;
		screen_left = 0.f;// *-0.5;
		screen_top = 0.f;// (float)h * -0.5;
		screen_right = (float)w;// *0.5;
		screen_bottom = (float)h;// *0.5;
		zoom_factor = 1.f;
		m_points = 0;
		m_portal.setIsPortal(false);
		passed_level = false;
		m_level = 0;
		m_game_level = 0;
		initTrees();
		used_skillpoints = 0;
		skill_num = 0;
		item_num = 0;
		ice_skill_set = { 0.f,0.f,0.f };
		thunder_skill_set = { 0.f,0.f,0.f };
		fire_skill_set = { 0.f,0.f,0.f };
		level_num = { 0.f,0.f,1.f };
		previous_point = 0;
		pass_points = 5;
		map.set_is_over(true);
		start_is_over = false;
		game_is_paused = false;
		shopping = false;
		display_tutorial = false;
		cur_points_needed = pass_points - m_points;
		kill_num = number_to_vec(cur_points_needed, true);
	}

	// Control the current speed with `<` `>`
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_COMMA)
		m_current_speed -= 0.1f;
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_PERIOD)
		m_current_speed += 0.1f;

	//add toggle
	//if (action == GLFW_RELEASE && key == GLFW_KEY_1) {
	//	m_hero.advanced = true;
	//}
	//if (action == GLFW_RELEASE && key == GLFW_KEY_B) {
	//	m_hero.advanced = false;
	//}

	m_current_speed = fmax(0.f, m_current_speed);

	//add key control for direction
	vec2 cur_direction = m_hero.get_direction();

	if (action == GLFW_PRESS && key == GLFW_KEY_D) {
		m_hero.set_direction({ 1.0f,cur_direction.y });
	}
	else if (action == GLFW_RELEASE && key == GLFW_KEY_D && m_hero.get_direction().x > 0)
	{
		m_hero.set_direction({ 0.f,cur_direction.y });
	}

	if (action == GLFW_PRESS && key == GLFW_KEY_A) {
		m_hero.set_direction({ -1.0f,cur_direction.y });
	}
	else if (action == GLFW_RELEASE && key == GLFW_KEY_A && m_hero.get_direction().x < 0)
	{
		m_hero.set_direction({ 0.f,cur_direction.y });
	}


	cur_direction = m_hero.get_direction();
	if (action == GLFW_PRESS && key == GLFW_KEY_W) {
		m_hero.set_direction({ cur_direction.x,-1.0 });
	}
	else if (action == GLFW_RELEASE && key == GLFW_KEY_W && m_hero.get_direction().y < 0)
	{
		m_hero.set_direction({ cur_direction.x,0.f });
	}

	if (action == GLFW_PRESS && key == GLFW_KEY_S) {
		m_hero.set_direction({ cur_direction.x,1.0f });
	}
	else if (action == GLFW_RELEASE && key == GLFW_KEY_S && m_hero.get_direction().y > 0)
	{
		m_hero.set_direction({ cur_direction.x,0.f });
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
	else if (key == GLFW_KEY_G && start_is_over == false && !shopping) {
		map.init(screen,m_game_level);
		start_is_over = true;
		zoom_factor = 1.1f;
	}
	else if (key == GLFW_KEY_ESCAPE && action != GLFW_RELEASE && !start_is_over) {
        if (!display_tutorial && !shopping) {
            // escape in start screen
            glfwSetWindowShouldClose(m_window, GL_TRUE);
        }
		else if (shopping) {
			shopping = !shopping;
		}
        else {
            // escape in tutorial
            display_tutorial = !display_tutorial;
        }
	}
    else if (key == GLFW_KEY_ESCAPE && action != GLFW_RELEASE && start_is_over) {
        if (game_is_paused) {
            zoom_factor = 1.1f;
        }
        else {
            zoom_factor = 1.f;
        }
        game_is_paused = !game_is_paused;
    }
	else if (key == GLFW_KEY_SPACE && action != GLFW_RELEASE && start_is_over) {
		if (game_is_paused) {
			zoom_factor = 1.1f;
		}
		else {
			zoom_factor = 1.f;
		}
		game_is_paused = !game_is_paused;
	}
	else if (key == GLFW_KEY_E && action == GLFW_RELEASE) {
		m_hero.set_active_skill(0);
	}
	else if (key == GLFW_KEY_Q && action == GLFW_RELEASE) {
		m_hero.set_active_skill(1);
	}
	else if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE) {
		m_hero.set_active_skill(2);
	}
}

void World::on_mouse_move(GLFWwindow* window, double xpos, double ypos)
{

	if (start_is_over && !game_is_paused && !shopping) {
		float angle = 0.f;
		vec2 salmon_position = m_hero.get_position();
		if (xpos - salmon_position.x != 0)
			angle = atan2((ypos - salmon_position.y), (xpos - salmon_position.x));

		m_hero.set_rotation(angle);
		mouse_position.x = float(xpos);
		mouse_position.y = float(ypos);
	}

	mouse_pos = { (float)xpos,(float)ypos };
}

void World::on_mouse_click(GLFWwindow* window, int button, int action, int mods)
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	vec2 screen = { (float)w, (float)h };
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE && !start_is_over ) {
		if (!display_tutorial && !shopping) {
			button_play.check_click(mouse_pos);
			button_tutorial.check_click(mouse_pos);
			button_shop.check_click(mouse_pos);
		}
		else {
			button_back_to_menu.check_click(mouse_pos);
			button_back_to_menu2.check_click(mouse_pos);
		}

	}
	if (shopping) {
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && item_num == 0) {
			item_num = shop_screen.item_position(mouse_pos, screen);
		}
		else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && item_num != 0) {
			if (shop_screen.level_position(mouse_pos, screen)) {
				std::string item_name = find_item(item_num);
				shop.buy_item(item_name);
				current_stock = shop.get_stock(item_name);
				current_price = shop.get_price(item_name);
				balance = shop.get_balance();
				item_num = 0;
			}
			else {
				item_num = shop_screen.item_position(mouse_pos, screen);
			}
		}
	}

	else if (!game_is_paused && start_is_over && !shopping) {
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
			shootingFireBall = true;

		}
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
			shootingFireBall = false;
		}

		if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
			m_hero.use_skill(hero_projectiles, thunders,phoenix_list,mouse_position);
			if (m_hero.get_active_skill() == THUNDER_SKILL)
				Mix_PlayChannel(-1, m_lightning_sound, 0);
			else if(m_hero.get_active_skill() == ICE_SKILL)
				Mix_PlayChannel(-1, m_ice_sound, 0);

		}


	}
	else if (game_is_paused && start_is_over && !shopping) {

		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && skill_num == 0 && skill_element != stree.element_position(mouse_pos)) {
			skill_element = stree.element_position(mouse_pos);
			if (skill_element == "ice") {
				stree.init(screen, 1);
			}
			else if (skill_element == "thunder") {
				stree.init(screen, 2);
			}
			else if (skill_element == "fire") {
				stree.init(screen, 3);
			}
		}
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && skill_num == 0) {
			skill_num = stree.icon_position(mouse_pos, skill_element);
			if (skill_num == 1 && skill_element == "ice" && ice_skill_set.x == 5.f) {
				skill_num = 0;
			}else if (skill_num == 1 && skill_element == "thunder" && thunder_skill_set.x == 5.f) {
				skill_num = 0;
			}else if (skill_num == 2 && skill_element == "ice" && ice_skill_set.y == 5.f) {
				skill_num = 0;
			}else if (skill_num == 2 && skill_element == "thunder" && thunder_skill_set.y == 5.f) {
				skill_num = 0;
			}else if (skill_num == 3 && skill_element == "ice" && ice_skill_set.z == 5.f) {
				skill_num = 0;
			}else if (skill_num == 3 && skill_element == "thunder" && thunder_skill_set.z == 5.f) {
				skill_num = 0;
			}
			else if (skill_num == 1 && skill_element == "fire" && fire_skill_set.x == 5.f) {
				skill_num = 0;
			}
			else if (skill_num == 2 && skill_element == "fire" && fire_skill_set.y == 5.f) {
				skill_num = 0;
			}
			else if (skill_num == 3 && skill_element == "fire" && fire_skill_set.z == 5.f) {
				skill_num = 0;
			}
		}
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && skill_num == 1) {
			if (stree.level_position(mouse_pos) && m_level > used_skillpoints) {
				used_skillpoints++;
				skill_num = 0;
				if (skill_element == "ice") {
					ice_skill_set.x = ice_skill_set.x + 1.f;
					m_hero.level_up(0, 1);
				}
				else if (skill_element == "thunder") {
					thunder_skill_set.x = thunder_skill_set.x + 1.f;
					m_hero.level_up(1, 0);
				}
				else if (skill_element == "fire") {
					fire_skill_set.x = fire_skill_set.x + 1.f;
					m_hero.level_up(2, 0);
				}
			}
			else {
				skill_num = stree.icon_position(mouse_pos, skill_element);
			}
		}
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && skill_num == 2) {
			if (stree.level_position(mouse_pos) && m_level > used_skillpoints) {
				used_skillpoints++;
				skill_num = 0;
				if (skill_element == "ice") {
					ice_skill_set.y = ice_skill_set.y + 1.f;
					m_hero.level_up(0, 2);
				}
				else if (skill_element == "thunder") {
					thunder_skill_set.y = thunder_skill_set.y + 1.f;
					m_hero.level_up(1, 1);
				}
				else if (skill_element == "fire") {
					fire_skill_set.y = fire_skill_set.y + 1.f;
					m_hero.level_up(2, 1);
				}
			}
			else {
				skill_num = stree.icon_position(mouse_pos, skill_element);
			}
		}
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && skill_num == 3) {
			if (stree.level_position(mouse_pos) && m_level > used_skillpoints) {
				used_skillpoints++;
				skill_num = 0;
				if (skill_element == "ice") {
					ice_skill_set.z = ice_skill_set.z + 1.f;
					m_hero.level_up(0, 0);
				}
				else if (skill_element == "thunder") {
					thunder_skill_set.z = thunder_skill_set.z + 1.f;
					m_hero.level_up(1, 2);
				}
				else if (skill_element == "fire") {
					fire_skill_set.z = fire_skill_set.z + 1.f;
					m_hero.level_up(2, 2);
				}
			}
			else {
				skill_num = stree.icon_position(mouse_pos, skill_element);
			}
		}
		if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS && start_is_over && !shopping) {
			m_hero.use_skill(hero_projectiles, thunders,phoenix_list,mouse_position);
		}
	}
}


void World::on_mouse_wheel(GLFWwindow* window, double xoffset, double yoffset)
{
	if (yoffset < -0.f)
	{
		int level_up_skill = (m_hero.get_active_skill() - 1 + 3) % 3;
		m_hero.set_active_skill(level_up_skill);
	}
	else if (yoffset > 0.f)
	{
		int level_up_skill = (m_hero.get_active_skill() + 1) % 3;
		m_hero.set_active_skill(level_up_skill);
	}

}

void World::startGame()
{
	// grab screen size first
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	vec2 screen = { (float)w, (float)h };

	// input code from key input, "G"
	if (start_is_over == false) {
		map.init(screen,m_game_level);
		start_is_over = true;
		zoom_factor = 1.1f;
	}
	else {
		fprintf(stderr, "WORLD::STARTGAME CALLED WHEN start_is_over IS NOT FALSE!");
	}
	return;
}

vec3 World::number_to_vec(int number, bool kill)
{
	float g = 0.f;
	float s = 0.f;
	float b = 0.f;

	if (kill) {
		while (number > 99) {
			b += 1.f;
			number -= 100.f;
		}

		while (number > 9) {
			s += 1.f;
			number -= 10.f;
		}

		while (number > 0) {
			g += 1.f;
			number -= 1.f;
		}

		return { b,s,g };
	}
	else {
		if (number % 2 == 0) {
			g = 1.f;
		}
		else {
			g = 2.f;
		}
		number = floor(number/2);
		while (number > 9) {
			b += 1.f;
			number -= 10.f;
		}

		while (number > 0) {
			s += 1.f;
			number -= 1.f;
		}
		return { b,s,g };
	}
}
std::string World::find_item(int item_num) {
	std::string result = "";
	switch (item_num) {
	case 1:
		result = "max_hp";
		break;
	case 2:
		result = "mp_recovery";
		break;
	case 3:
		result = "exp_increase";
		break;
	case 4:
		result = "fireball_damage";
		break;
	case 5:
		result = "movement_speed";
		break;
	case 6:
		result = "second_life";
		break;
	}
	return result;
}
void World::doNothing() {
	// NOT A STUB
	return;
}

