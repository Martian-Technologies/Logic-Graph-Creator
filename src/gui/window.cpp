#include "window.h"

Window::Window()
	: sdlWindow("Gatality") {
	
}


bool Window::recieveEvent(const SDL_Event& event) {
	if (sdlWindow.isThisMyEvent(event)) {
		return true;
	}
	return false;
}
