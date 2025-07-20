#ifndef evalWindow_h
#define evalWindow_h

#include "backend/blockData/blockDataManager.h"
#include "backend/tools/toolManagerManager.h"
#include "gui/helper/menuTree.h"

class EvaluatorManager;
class CircuitManager;
class CircuitViewWidget;

class EvalWindow {
public:
	EvalWindow(
		const EvaluatorManager* evaluatorManager,
		const CircuitManager* circuitManager,
		std::shared_ptr<CircuitViewWidget> circuitViewWidget,
		DataUpdateEventManager* dataUpdateEventManager,
		Rml::ElementDocument* document,
		Rml::Element* parent
	);

	void updateList();

private:
	void updateSelected(std::string string);
	void makePaths(std::vector<std::vector<std::string>>& paths, std::vector<std::string>& path, const AddressTreeNode<Evaluator::EvaluatorGate>& addressTree);

	MenuTree menuTree;
	DataUpdateEventManager::DataUpdateEventReceiver dataUpdateEventReceiver;
	std::shared_ptr<CircuitViewWidget> circuitViewWidget;
	const EvaluatorManager* evaluatorManager;
	const CircuitManager* circuitManager;

};

#endif /* evalWindow_h */