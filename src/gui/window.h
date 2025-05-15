#ifndef window_h
#define window_h

#include <RmlUi/Core.h>
#include <SDL3/SDL_events.h>

#include "sdl/sdlWindow.h"
#include "gpu/renderer/windowRenderer.h"
#include "rml/rmlRenderInterface.h"

#include "backend/backend.h"
#include "computerAPI/circuits/circuitFileManager.h"

#include "selectorWindow.h"
#include "evalWindow.h"
#include "blockCreationWindow.h"
#include "circuitViewWidget.h"
#include "gui/circuitView/simControlsManager.h"

class Window {
public:
	Window(Backend* backend, CircuitFileManager* circuitFileManager, RmlRenderInterface& rmlRenderInterface, VulkanInstance* vulkanInstance);
	~Window();

	// no copy
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;
	
public:
	bool recieveEvent(SDL_Event& event);
	void updateRml(RmlRenderInterface& renderInterface);

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
	Backend* backend;
	CircuitFileManager* circuitFileManager;

	SdlWindow sdlWindow;
	WindowRenderer renderer;

	std::optional<SelectorWindow> selectorWindow;
	std::optional<EvalWindow> evalWindow;
	std::optional<BlockCreationWindow> blockCreationWindow;
	std::optional<SimControlsManager> simControlsManager;

	std::shared_ptr<CircuitViewWidget> circuitViewWidget;
	
	Rml::Context* rmlContext;
	Rml::ElementDocument* rmlDocument;
};

#endif
