#ifndef blockCreationWindow_h
#define blockCreationWindow_h

#include "gui/mainWindow/tools/toolManagerManager.h"
#include <RmlUi/Core.h>

class CircuitManager;
class MainWindow;

class BlockCreationWindow {
public:
	BlockCreationWindow(
		CircuitManager* circuitManager,
		MainWindow* mainWindow,
		DataUpdateEventManager* dataUpdateEventManager,
		ToolManagerManager* toolManagerManager,
		Rml::ElementDocument* document,
		Rml::Element* menu
	);

	void updateFromMenu();
	void resetMenu();

private:
	void addListItem(bool isInput);
	void updateSelected(std::string string);
	void makePaths(std::vector<std::vector<std::string>>& paths, std::vector<std::string>& path, const EvalAddressTree& addressTree);

	Rml::ElementDocument* document;
	Rml::Element* outputList;
	Rml::Element* inputList;
	Rml::Element* menu;
	DataUpdateEventManager::DataUpdateEventReceiver dataUpdateEventReceiver;
	MainWindow* mainWindow;
	CircuitManager* circuitManager;
	ToolManagerManager* toolManagerManager;
};

#endif /* blockCreationWindow_h */