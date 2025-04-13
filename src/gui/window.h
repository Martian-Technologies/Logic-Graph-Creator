#ifndef window_h
#define window_h

#include <RmlUi/Core.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_render.h>

#include "gui/rml/RmlUi_Renderer_SDL.h"
#include "sdl/sdlWindow.h"

#include "backend/backend.h"
#include "computerAPI/circuits/circuitFileManager.h"

#include "selectorWindow.h"
#include "circuitViewWidget.h"

class Window {
public:
	Window(Backend* backend, CircuitFileManager* circuitFileManager);
	~Window();

	bool recieveEvent(SDL_Event& event);
	void update();
	void render(RenderInterface_SDL& renderInterface);

	inline SDL_Window* getSdlWindow() { return sdlWindow.getHandle(); };
	inline std::shared_ptr<CircuitViewWidget> getCircuitViewWidget() { return circuitViewWidget; };

	void saveCircuit(circuit_id_t id, bool saveAs);
	void loadCircuit();
	void exportProject();
	void setBlock(std::string blockPath);
	void setTool(std::string tool);
	void setMode(std::string tool);

private:
	std::shared_ptr<CircuitViewWidget> circuitViewWidget;
	Backend* backend;
	CircuitFileManager* circuitFileManager;
	std::optional<SelectorWindow> selectorWindow;

	SdlWindow sdlWindow;
	SDL_Renderer* sdlRenderer;
	 
	Rml::Context* rmlContext;
};

#endif
