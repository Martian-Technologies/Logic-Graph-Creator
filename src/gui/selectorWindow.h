#ifndef selectorWindow_h
#define selectorWindow_h

#include "backend/blockData/blockDataManager.h"
#include "interaction/menuTree.h"

class SelectorWindow {
public:
	SelectorWindow(
		const BlockDataManager* blockDataManager,
		DataUpdateEventManager* dataUpdateEventManager,
		Rml::ElementDocument* document,
		Rml::Element* parent
	);
	
	void updateToolModeOptions(const std::vector<std::string>* modes);
	void updateBlockList();

private:
	void updateSelected();
	void updateSelectedMode(QListWidgetItem *current, QListWidgetItem *previous);

	const BlockDataManager* blockDataManager;
	MenuTree menuTree;
	DataUpdateEventManager::DataUpdateEventReceiver dataUpdateEventReceiver;
};

#endif /* selectorWindow_h */