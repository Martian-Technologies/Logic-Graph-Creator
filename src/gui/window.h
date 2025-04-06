#ifndef window_h
#define window_h

#include <RmlUi/Core.h>
#include <SDL3/SDL_events.h>

#include "sdl/sdlWindow.h"
#include "gpu/renderer/windowRenderer.h"
#include "rml/rmlRenderInterface.h"

#include "backend/backend.h"
#include "computerAPI/circuits/circuitFileManager.h"

class Window {
public:
	Window(Backend* backend, CircuitFileManager* circuitFileManager);
	~Window();

	// no copy
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;
	
public:
	bool recieveEvent(SDL_Event& event);
	void updateRml(RmlRenderInterface& renderInterface);

	inline SDL_Window* getSdlWindow() { return sdlWindow.getHandle(); };

    void saveCircuit(circuit_id_t id, bool saveAs);
    void loadCircuit();
    void exportProject();
	void setBlock(std::string blockPath);
	void setTool(std::string tool);
	void setMode(std::string tool);

private:
	Backend* backend;
	CircuitFileManager* circuitFileManager;

	SdlWindow sdlWindow;
	WindowRenderer renderer;
	 
	Rml::Context* rmlContext;
};

#endif
