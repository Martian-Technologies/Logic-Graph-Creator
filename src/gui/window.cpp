#include "window.h"

Window::Window()
	: sdlWindow("Gatality") {
	
}

void Window::runLoop() {
	running = true;
	while (running) {
		std::vector<SDL_Event> events = sdlWindow.pollEvents();
		for (const SDL_Event& event : events) {
			if (event.type == SDL_EVENT_QUIT) {
				running = false;
			}
		}
	}
}
