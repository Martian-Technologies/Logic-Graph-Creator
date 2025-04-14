#ifndef evalWindow_h
#define evalWindow_h

#include "backend/blockData/blockDataManager.h"
#include "backend/tools/toolManagerManager.h"
#include "interaction/menuTree.h"

class EvaluatorManager;
class CircuitManager;


class EvalWindow {
public:
	EvalWindow(
		const EvaluatorManager* evaluatorManager,
		const CircuitManager* circuitManager,
		DataUpdateEventManager* dataUpdateEventManager,
		Rml::ElementDocument* document,
		Rml::Element* parent
	);

	void updateToolModeOptions(const std::vector<std::string>* modes);
	void updateList();

private:
	void updateSelected(std::string string);
	void updateSelectedMode();
	void makePaths(std::vector<std::vector<std::string>>& paths, std::vector<std::string>& path, const AddressTreeNode<Evaluator::EvaluatorGate>& addressTree);

	MenuTree menuTree;
	DataUpdateEventManager::DataUpdateEventReceiver dataUpdateEventReceiver;
	const EvaluatorManager* evaluatorManager;
	const CircuitManager* circuitManager;

};

#endif /* evalWindow_h */