#include "selectorWindow.h"
#include "backend/dataUpdateEventManager.h"
#include "gui/helper/eventPasser.h"
#include "util/algorithm.h"
#include "backend/circuitView/tools/placement/blockPlacementTool.h"

SelectorWindow::SelectorWindow(
	const BlockDataManager* blockDataManager,
	DataUpdateEventManager* dataUpdateEventManager,
	ProceduralCircuitManager* proceduralCircuitManager,
	ToolManagerManager* toolManagerManager,
	Rml::ElementDocument* document
) : blockDataManager(blockDataManager), proceduralCircuitManager(proceduralCircuitManager), toolManagerManager(toolManagerManager), dataUpdateEventReceiver(dataUpdateEventManager), document(document) {
	Rml::Element* itemTreeParent = document->GetElementById("item-selection-tree");
	menuTree.emplace(document, itemTreeParent, false);
	menuTree->setListener(std::bind(&SelectorWindow::updateSelected, this, std::placeholders::_1));
	// In Blocks/Tools tree, prevent selecting category dropdown parents ("Blocks" / "Tools")
	menuTree->disallowParentSelection(true);
	dataUpdateEventReceiver.linkFunction("blockDataUpdate", [this](const DataUpdateEventManager::EventData*) { refreshSidebar(true); });
	dataUpdateEventReceiver.linkFunction("proceduralCircuitPathUpdate", [this](const DataUpdateEventManager::EventData*) { refreshSidebar(true); });

	Rml::Element* modeTreeParent = document->GetElementById("mode-selection-tree");
	modeMenuTree.emplace(document, modeTreeParent, false);
	modeMenuTree->setListener(std::bind(&SelectorWindow::updateSelectedMode, this, std::placeholders::_1));
	dataUpdateEventReceiver.linkFunction("setToolUpdate", [this](const DataUpdateEventManager::EventData*) { refreshSidebar(false); });

	parameterMenu = document->GetElementById("parameter-menu");
	parameterMenu->GetElementById("reset-parameters")->AddEventListener(Rml::EventId::Click, new EventPasser([this](Rml::Event& event) {setupProceduralCircuitParameterMenu();}));
	parameterMenu->GetElementById("create-block")->AddEventListener(Rml::EventId::Click, new EventPasser(
		[this](Rml::Event& event) {
			if (!selectedProceduralCircuit) return;
			Rml::Element* parametersElement = parameterMenu->GetElementById("parameter-menu-parameters");

			ProceduralCircuitParameters proceduralCircuitParameters;
			for (unsigned int i = 0; i < parametersElement->GetNumChildren(); ++i) {
				Rml::ElementList elements;
				parametersElement->GetChild(i)->GetElementsByClassName(elements, "parameter-name");
				std::string key = elements[0]->GetInnerRML();
				key.pop_back();
				elements.clear();
				parametersElement->GetChild(i)->GetElementsByClassName(elements, "parameter-input");
				Rml::ElementFormControlInput* parameterInput = rmlui_dynamic_cast<Rml::ElementFormControlInput*>(elements[0]);
				std::string str = parameterInput->GetValue();
				try {
					int value = std::stoi(str);
					proceduralCircuitParameters.parameters.emplace(std::move(key), value);
				} catch (std::exception const& ex) {
					logError("Invalid parameter for {}: {}. {}", "", key, str, ex.what());
					return;
				}
			}
			this->toolManagerManager->setBlock(selectedProceduralCircuit->getBlockType(proceduralCircuitParameters));
		}
	));

	refreshSidebar(true);
}

void SelectorWindow::updateList() {
	std::vector<std::vector<std::string>> paths;
	for (unsigned int blockType = 1; blockType <= blockDataManager->maxBlockId(); blockType++) {
		if (!blockDataManager->isPlaceable((BlockType)blockType)) continue;
		std::vector<std::string>& path = paths.emplace_back(1, "Blocks");
		stringSplitInto(blockDataManager->getPath((BlockType)blockType), '/', path);
		path.push_back(blockDataManager->getName((BlockType)blockType));
	}
	for (const auto& iter : toolManagerManager->getAllTools()) {
		std::vector<std::string>& path = paths.emplace_back(1, "Tools");
		stringSplitInto(iter.first, '/', path);
	}
	for (const auto& iter : proceduralCircuitManager->getProceduralCircuits()) {
		std::vector<std::string>& path = paths.emplace_back(1, "Blocks");
		stringSplitInto(iter.second->getPath(), '/', path);
	}
	menuTree->setPaths(paths);
}

void SelectorWindow::updateToolModeOptions() { refreshSidebar(false); }

void SelectorWindow::refreshSidebar(bool rebuildItems) {
	if (rebuildItems) updateList();
	auto modes = toolManagerManager->getActiveToolModes();
	modeMenuTree->setPaths(modes.value_or(std::vector<std::string>()));
	highlightActiveToolInSidebar();
	applyAndHighlightActiveMode();
}

void SelectorWindow::highlightActiveToolInSidebar() {
	if (!menuTree) return;
	const std::string& activeTool = toolManagerManager->getActiveTool();
	logInfo("Active tool changed to: {}", "", activeTool);
	if (activeTool.empty()) return;
	std::string activeToolId = std::string("Tools/") + activeTool + "-menu";
	logInfo("Highlighting active tool in sidebar: {}", "", activeToolId);
	if (Rml::Element* activeEl = document->GetElementById(activeToolId)) {
		if (Rml::Element* itemRoot = document->GetElementById("item-selection-tree")) {
			Rml::ElementList rows; itemRoot->GetElementsByTagName(rows, "li");
			for (auto* r : rows) r->SetClass("selected", false);
		}
		activeEl->SetClass("selected", true);
		// Expand ancestors
		Rml::Element* p = activeEl->GetParentNode();
		while (p) {
			if (p->GetTagName() == "li") p->SetClass("collapsed", false);
			p = p->GetParentNode();
		}
	}
	if (activeTool == "placement") {
		// get the block placement tool
		SharedCircuitTool blockPlacementTool = toolManagerManager->getToolInstance();
		if (blockPlacementTool) {
			SharedBlockPlacementTool placementTool = std::dynamic_pointer_cast<BlockPlacementTool>(blockPlacementTool);
			if (placementTool) {
				BlockType selected = placementTool->getSelectedBlock();
				if (selected != BlockType::NONE) {
					// Build the element id used by MenuTree: "Blocks/<path>/<name>-menu"
					std::string blockPath = blockDataManager->getPath(selected);
					std::string blockName = blockDataManager->getName(selected);
					std::string elementId = "Blocks/";
					if (!blockPath.empty()) elementId += blockPath + "/";
					elementId += blockName + "-menu";
					if (Rml::Element* blockEl = document->GetElementById(elementId)) {
						// Highlight and ensure its parents are expanded
						blockEl->SetClass("selected", true);
						Rml::Element* p = blockEl->GetParentNode();
						while (p) {
							if (p->GetTagName() == "li") p->SetClass("collapsed", false);
							p = p->GetParentNode();
						}
					}
				}
			}
		}
	}
}

void SelectorWindow::applyAndHighlightActiveMode() {
	auto modes = toolManagerManager->getActiveToolModes();
	if (!(modes && !modes->empty())) return;
	std::string modeToApply;
	if (auto stored = toolManagerManager->getActiveToolStoredMode(); stored && std::find(modes->begin(), modes->end(), *stored) != modes->end()) {
		modeToApply = *stored;
	} else {
		modeToApply = (*modes)[0];
	}
	toolManagerManager->setMode(modeToApply);
	if (!modeMenuTree) return;
	logInfo("Highlighting active mode in sidebar: {}", "", modeToApply);
	if (Rml::Element* modeRoot = document->GetElementById("mode-selection-tree")) {
		Rml::ElementList rows; modeRoot->GetElementsByTagName(rows, "li");
		for (auto* r : rows) r->SetClass("selected", false);
		for (auto* r : rows) {
			std::string id = r->GetId();
			if (id.size() >= 5) {
				std::string path = id.substr(0, id.size() - 5);
				if (path == modeToApply) { r->SetClass("selected", true); break; }
			}
		}
	}
}


void SelectorWindow::updateSelected(const std::string& string) {
	std::vector parts = stringSplit(string, '/');
	if (parts.size() <= 1) return;
	if (parts[0] == "Blocks") {
		selectedProceduralCircuit = nullptr; // either it will be set or this should go away!
		std::string path = string.substr(7, string.size() - 7);
		BlockType blockType = blockDataManager->getBlockType(path);
		if (blockType == BlockType::NONE) {
			const std::string* uuid = proceduralCircuitManager->getProceduralCircuitUUID(path);
			if (uuid) {
				selectedProceduralCircuit = proceduralCircuitManager->getProceduralCircuit(*uuid);
				if (selectedProceduralCircuit) setupProceduralCircuitParameterMenu();
				else logError("unknown block with path: {}", "SelectorWindow", path);
			}
		}
	toolManagerManager->setBlock(blockType);
	if (!selectedProceduralCircuit) hideProceduralCircuitParameterMenu();
	} else if (parts[0] == "Tools") {
		std::string toolPath = string.substr(6, string.size() - 6);
		toolManagerManager->setTool(toolPath);
	} else {
		logError("Do not recognize cadegory {}", "SelectorWindow", parts[0]);
	}
	refreshSidebar(false);
}

void SelectorWindow::updateSelectedMode(const std::string& string) {
	toolManagerManager->setMode(string);
}

void SelectorWindow::setupProceduralCircuitParameterMenu() {
	if (!selectedProceduralCircuit) {
		hideProceduralCircuitParameterMenu();
		return;
	}
	parameterMenu->GetParentNode()->SetClass("invisible", false);
	parameterMenu->GetElementById("parameter-menu-active")->SetInnerRML(selectedProceduralCircuit->getProceduralCircuitName());
	Rml::Element* parametersElement = parameterMenu->GetElementById("parameter-menu-parameters");

	while (parametersElement->GetNumChildren() > 0) parametersElement->RemoveChild(parametersElement->GetChild(0));

	const ProceduralCircuitParameters& parameters = selectedProceduralCircuit->getParameterDefaults();
	for (const std::pair<std::string, int>& pair : parameters.parameters) {
		Rml::ElementPtr parameterDiv = document->CreateElement("div");
		parameterDiv->SetClass("parameter", true);

		Rml::ElementPtr parameterNameElement = document->CreateElement("span");
		parameterNameElement->SetInnerRML(pair.first + ":");
		parameterNameElement->SetClass("parameter-name", true);

		Rml::XMLAttributes parameterInputAttributes;
		parameterInputAttributes["type"] = "text";
		parameterInputAttributes["maxlength"] = "8";
		parameterInputAttributes["size"] = "8";
		Rml::ElementPtr parameterInputElement = Rml::Factory::InstanceElement(document, "input", "input", parameterInputAttributes);
		parameterInputElement->SetClass("parameter-input", true);
		Rml::ElementFormControlInput* parameterInput = rmlui_dynamic_cast<Rml::ElementFormControlInput*>(parameterInputElement.get());
		parameterInput->SetValue(std::to_string(pair.second));

		parameterDiv->AppendChild(std::move(parameterNameElement));
		parameterDiv->AppendChild(std::move(parameterInputElement));
		parametersElement->AppendChild(std::move(parameterDiv));
	}
}

void SelectorWindow::hideProceduralCircuitParameterMenu() {
	parameterMenu->GetParentNode()->SetClass("invisible", true);
}
