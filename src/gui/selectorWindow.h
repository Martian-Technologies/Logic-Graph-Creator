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

	void updateToolModeOptions();
	void updateList();

private:
	void updateSelected(const std::string& string);
	void updateSelectedMode(const std::string& string);

	std::optional<MenuTree> menuTree;
	std::optional<MenuTree> modeMenuTree;
	const BlockDataManager* blockDataManager;
	ToolManagerManager* toolManagerManager;
	DataUpdateEventManager::DataUpdateEventReceiver dataUpdateEventReceiver;
};

#endif /* selectorWindow_h */