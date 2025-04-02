#ifndef sdlWindow_h
#define sdlWindow_h

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>

class SdlWindow {
public:
	SdlWindow(const std::string& name);
	~SdlWindow();

	bool isThisMyEvent(const SDL_Event& event);
	VkSurfaceKHR createSurface(VkInstance instance);
	
	inline SDL_Window* getHandle() { return handle; }

private:
	// TODO - smart pointer with custom deleter?
	SDL_Window* handle;
};

#endif
