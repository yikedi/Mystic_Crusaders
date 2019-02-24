#pragma once

#include <vector>

#include "world.hpp"


class ScreenManager
{
	public:

		ScreenManager();
		~ScreenManager();
		
		// Create our current window, and load starting song for stage 0 (main menu)
		bool init();

		// Changes our current level to the one we need
		void changeLevel(int levelNumber);

		// Adds one screen to the top of the stack (simulated with a vector)
		void addScreen(int screenNumber);

		// Dequeues the last screen from our thing
		void popScreen();

		// Updates all items on vector
		void update(float ms);

		// Draws all items on vector, in the correct order
		void draw();

		// Grabs our current window
		GLFWwindow* getWindow();

		// Returns our current rendering stack
		std::vector<int> getStack();

	private:

		int levelNumber; // level we're currently on
		int screenNumber; // number of screens we have; tbd whether to keep
		std::vector<int> renderableStack; // things to load

};