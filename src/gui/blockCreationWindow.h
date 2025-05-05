#ifndef blockCreationWindow_h
#define blockCreationWindow_h

#include "backend/blockData/blockDataManager.h"
#include "backend/tools/toolManagerManager.h"
#include "interaction/menuTree.h"
#include "backend/tools/toolManagerManager.h"

class CircuitManager;
class CircuitViewWidget;

class BlockCreationWindow {
public:
	BlockCreationWindow(
		CircuitManager* circuitManager,
		std::shared_ptr<CircuitViewWidget> circuitViewWidget,
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
	void makePaths(std::vector<std::vector<std::string>>& paths, std::vector<std::string>& path, const AddressTreeNode<Evaluator::EvaluatorGate>& addressTree);

	Rml::ElementDocument* document;
	Rml::Element* outputList;
	Rml::Element* inputList;
	Rml::Element* menu;
	DataUpdateEventManager::DataUpdateEventReceiver dataUpdateEventReceiver;
	std::shared_ptr<CircuitViewWidget> circuitViewWidget;
	CircuitManager* circuitManager;
	ToolManagerManager* toolManagerManager;
};

#endif /* blockCreationWindow_h */