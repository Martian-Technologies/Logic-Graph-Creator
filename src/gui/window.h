#ifndef window_h
#define window_h

#include <RmlUi/Core.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_render.h>

#include "gui/rml/RmlUi_Renderer_SDL.h"
#include "sdl/sdlWindow.h"

class Window {
public:
	Window();
	~Window();

	bool recieveEvent(SDL_Event& event);
	void update();
	void render(RenderInterface_SDL& renderInterface);

	inline SDL_Window* getSdlWindow() { return sdlWindow.getHandle(); };

private:
	SdlWindow sdlWindow;
	SDL_Renderer* sdlRenderer;
	 
	Rml::Context* rmlContext;
};

#endif
