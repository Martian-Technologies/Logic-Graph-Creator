#ifndef evalWindow_h
#define evalWindow_h

#include "backend/blockData/blockDataManager.h"
#include "backend/tools/toolManagerManager.h"
#include "interaction/menuTree.h"

class EvaluatorManager;

class EvalWindow {
public:
	EvalWindow(
		const EvaluatorManager* evaluatorManager,
		DataUpdateEventManager* dataUpdateEventManager,
		Rml::ElementDocument* document,
		Rml::Element* parent
	);

	void updateToolModeOptions(const std::vector<std::string>* modes);
	void updateList();

private:
	void updateSelected(std::string string);
	void updateSelectedMode();

	MenuTree menuTree;
	DataUpdateEventManager::DataUpdateEventReceiver dataUpdateEventReceiver;
};

#endif /* evalWindow_h */