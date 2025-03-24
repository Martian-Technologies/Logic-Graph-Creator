#ifndef window_h
#define window_h

#include "sdl/sdlWindow.h"
#include <SDL3/SDL_events.h>

class Window {
public:
	Window();

	bool recieveEvent(const SDL_Event& event);

	inline bool createSurface(VkInstance& instance, VkSurfaceKHR* out_surface) { return sdlWindow.createSurface(instance, out_surface); }
	inline SDL_Window* getSdlWindow() { return sdlWindow.getHandle(); };

private:
	SdlWindow sdlWindow;
};

#endif
