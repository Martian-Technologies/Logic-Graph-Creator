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
		ProceduralCircuitManager* proceduralCircuitManager,
		ToolManagerManager* toolManagerManager,
		Rml::ElementDocument* document
	);

	void updateToolModeOptions();
	void updateList();

private:
	void updateSelected(const std::string& string);
	void updateSelectedMode(const std::string& string);
	void setupProceduralCircuitParameterMenu();
	void hideProceduralCircuitParameterMenu();

	SharedProceduralCircuit selectedProceduralCircuit = nullptr;

	Rml::ElementDocument* document;
	Rml::Element* parameterMenu;
	std::optional<MenuTree> menuTree;
	std::optional<MenuTree> modeMenuTree;
	const BlockDataManager* blockDataManager;
	ProceduralCircuitManager* proceduralCircuitManager;
	ToolManagerManager* toolManagerManager;
	DataUpdateEventManager::DataUpdateEventReceiver dataUpdateEventReceiver;
};

#endif /* selectorWindow_h */