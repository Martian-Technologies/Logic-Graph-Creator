#ifndef window_h
#define window_h

#include <RmlUi/Core.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_render.h>

#include "gui/rml/RmlUi_Renderer_SDL.h"
#include "sdl/sdlWindow.h"

#include "computerAPI/circuits/circuitFileManager.h"
#include "computerAPI/fileListener/fileListener.h"
#include "backend/backend.h"

#include "gui/circuitView/simControlsManager.h"
#include "blockCreationWindow.h"
#include "circuitViewWidget.h"
#include "selectorWindow.h"
#include "evalWindow.h"

class Window {
public:
	Window(Backend* backend, CircuitFileManager* circuitFileManager, FileListener* fileListener, Rml::EventId pinchEventId);
	~Window();

	bool recieveEvent(SDL_Event& event);
	void update();
	void render(RenderInterface_SDL& renderInterface);

	inline SDL_Window* getSdlWindow() { return sdlWindow.getHandle(); };
	inline float getSdlWindowScalingSize() const { return sdlWindow.getWindowScalingSize(); }
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
	FileListener* fileListener;
	std::optional<SelectorWindow> selectorWindow;
	std::optional<EvalWindow> evalWindow;
	std::optional<BlockCreationWindow> blockCreationWindow;
	std::optional<SimControlsManager> simControlsManager;

	SdlWindow sdlWindow;
	SDL_Renderer* sdlRenderer;

	Rml::EventId pinchEventId;
	Rml::Context* rmlContext;
};

#endif
