#include "start_screen.hpp"

#include <iostream>

#include <string.h>
#include <cassert>
#include <sstream>

#include <gl3w.h>

Startscreen::Startscreen()
{
}

Startscreen::~Startscreen()
{
}

bool Startscreen::init(vec2 screen) {
	return true;
}

void Startscreen::destroy() {
}

void Startscreen::draw() {

}
bool Startscreen::update() {
//	int w, h;
	//glfwGetFramebufferSize(m_window, &w, &h);
	//vec2 screen = { (float)w, (float)h };
	return true;
}
bool Startscreen::is_over()const
{
	return false; // glfwWindowShouldClose(m_window);
}