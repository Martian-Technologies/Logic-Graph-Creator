#include "sdlWindow.h"

SdlWindow::SdlWindow(const std::string& name) {
	logInfo("Creating SDL window...");
	handle = SDL_CreateWindow(name.c_str(), 800, 600, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
	if (!handle)
	{
		throwFatalError("SDL could not create window! SDL_Error: " + std::string(SDL_GetError()));
	}
}

SdlWindow::~SdlWindow() {
	logInfo("Destroying SDL window...");
	SDL_DestroyWindow(handle);
}

bool SdlWindow::isThisMyEvent(const SDL_Event& event) {
	return SDL_GetWindowFromEvent(&event) == handle;
}

VkSurfaceKHR SdlWindow::createSurface(VkInstance instance) {
	VkSurfaceKHR surface;
	if (!SDL_Vulkan_CreateSurface(handle, instance, nullptr, &surface)) {
		throwFatalError("SDL could not create vulkan surface! SDL_Error: " + std::string(SDL_GetError()));
	}
	
	return surface;
}
