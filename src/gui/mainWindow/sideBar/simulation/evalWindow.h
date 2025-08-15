#ifndef evalWindow_h
#define evalWindow_h

// #include "gui/viewPortManager/tools/toolManagerManager.h"
#include "backend/evaluator/evalAddressTree.h"
#include "gui/helper/menuTree.h"

class EvaluatorManager;
class CircuitManager;
class MainWindow;

class EvalWindow {
public:
	EvalWindow(
		const EvaluatorManager* evaluatorManager,
		const CircuitManager* circuitManager,
		MainWindow* mainWindow,
		DataUpdateEventManager* dataUpdateEventManager,
		Rml::ElementDocument* document,
		Rml::Element* parent
	);

	void updateList();

private:
	void updateSelected(std::string string);
	void makePaths(std::vector<std::vector<std::string>>& paths, std::vector<std::string>& path, const EvalAddressTree& addressTree);

	MenuTree menuTree;
	DataUpdateEventManager::DataUpdateEventReceiver dataUpdateEventReceiver;
	MainWindow* mainWindow;
	const EvaluatorManager* evaluatorManager;
	const CircuitManager* circuitManager;

};

#endif /* evalWindow_h */