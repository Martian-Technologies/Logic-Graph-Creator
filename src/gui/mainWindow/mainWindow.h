#ifndef window_h
#define window_h

#include <RmlUi/Core.h>
#include <SDL3/SDL_events.h>

#include "gui/sdl/sdlWindow.h"
#include "gpu/renderer/windowRenderer.h"
#include "gui/rml/rmlRenderInterface.h"

#include "computerAPI/circuits/circuitFileManager.h"
#include "computerAPI/fileListener/fileListener.h"
#include "backend/backend.h"

#include "sideBar/icEditor/blockCreationWindow.h"
#include "circuitView/simControlsManager.h"
#include "circuitView/circuitViewWidget.h"
#include "sideBar/selector/selectorWindow.h"
#include "sideBar/simulation/evalWindow.h"

class MainWindow {
public:
	MainWindow(Backend* backend, CircuitFileManager* circuitFileManager, FileListener* fileListener, RmlRenderInterface& rmlRenderInterface, VulkanInstance* vulkanInstance);
	~MainWindow();

	// no copy
	MainWindow(const MainWindow&) = delete;
	MainWindow& operator=(const MainWindow&) = delete;
	
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
	void addPopUp(const std::string& message, const std::vector<std::pair<std::string, std::function<void()>>>& options);

private:
	void createPopUp(const std::string& message, const std::vector<std::pair<std::string, std::function<void()>>>& options);

	Backend* backend;
	CircuitFileManager* circuitFileManager;
	FileListener* fileListener;
	SdlWindow sdlWindow;
	WindowRenderer renderer;
	std::optional<SelectorWindow> selectorWindow;
	std::optional<EvalWindow> evalWindow;
	std::optional<BlockCreationWindow> blockCreationWindow;
	std::optional<SimControlsManager> simControlsManager;

	std::shared_ptr<CircuitViewWidget> circuitViewWidget;
	
	Rml::Context* rmlContext;
	Rml::ElementDocument* rmlDocument;

	std::vector<std::pair<std::string,const std::vector<std::pair<std::string, std::function<void()>>>>> popUpsToAdd;
};

#endif
