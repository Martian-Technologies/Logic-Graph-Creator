#ifndef selectorWindow_h
#define selectorWindow_h

#include "backend/blockData/blockDataManager.h"
#include "backend/tools/toolManagerManager.h"
#include "interaction/menuTree.h"

class SelectorWindow {
public:
	SelectorWindow(
		const BlockDataManager* blockDataManager,
		DataUpdateEventManager* dataUpdateEventManager,
		ToolManagerManager* toolManagerManager,
		Rml::ElementDocument* document,
		Rml::Element* parent
	);

	void updateToolModeOptions(const std::vector<std::string>* modes);
	void updateBlockList();
	void updateToolList();

private:
	void updateSelected(std::string string);
	void updateSelectedMode();

	MenuTree menuTree;
	const BlockDataManager* blockDataManager;
	ToolManagerManager* toolManagerManager;
	DataUpdateEventManager::DataUpdateEventReceiver dataUpdateEventReceiver;
};

#endif /* selectorWindow_h */