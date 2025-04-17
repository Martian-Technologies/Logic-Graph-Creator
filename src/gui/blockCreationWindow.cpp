#include "blockCreationWindow.h"
#include "backend/dataUpdateEventManager.h"
#include "backend/circuit/circuitManager.h"
#include "circuitViewWidget.h"
#include "backend/backend.h"
#include "util/algorithm.h"
#include "interaction/eventPasser.h"

BlockCreationWindow::BlockCreationWindow(
	CircuitManager* circuitManager,
	std::shared_ptr<CircuitViewWidget> circuitViewWidget,
	DataUpdateEventManager* dataUpdateEventManager,
	ToolManagerManager* toolManagerManager,
	Rml::ElementDocument* document,
	Rml::Element* menu
) : document(document), dataUpdateEventReceiver(dataUpdateEventManager), circuitManager(circuitManager), circuitViewWidget(circuitViewWidget), toolManagerManager(toolManagerManager) {
	// enuTree(document, parent, true, false)
	this->menu = menu;
	list = menu->GetElementById("connection-list");
	// update
	Rml::Element* updateButton = menu->GetElementById("update-block-creation");
	updateButton->AddEventListener("click", new EventPasser(std::bind(&BlockCreationWindow::updateFromMenu, this)));
	// reset
	Rml::Element* resetButton = menu->GetElementById("reset-block-creation");
	resetButton->AddEventListener("click", new EventPasser(std::bind(&BlockCreationWindow::resetMenu, this)));
	dataUpdateEventReceiver.linkFunction("blockDataUpdate", std::bind(&BlockCreationWindow::resetMenu, this));
	resetMenu();
}

void BlockCreationWindow::updateFromMenu() {
	Circuit* circuit = circuitViewWidget->getCircuitView()->getCircuit();
	if (!circuit) return;
	circuit_id_t id = circuit->getCircuitId();
	const CircuitBlockData* circuitBlockData = circuitManager->getCircuitBlockDataManager()->getCircuitBlockData(id);
	BlockData* blockData = circuitManager->getBlockDataManager()->getBlockData(circuitBlockData->getBlockType());

	// we dont update till the end because setting data will cause the UI to update

	Rml::Element* ele = menu->GetElementById("name-input");
	Rml::ElementFormControlInput* nameElement = rmlui_dynamic_cast<Rml::ElementFormControlInput*>(ele);
	std::string name = nameElement->GetValue();

	if (!blockData) {
		// set data if nothing more to get
		circuit->setCircuitName(name);
		return;
	}

	ele = menu->GetElementById("width-input");
	Rml::ElementFormControlInput* widthElement = rmlui_dynamic_cast<Rml::ElementFormControlInput*>(ele);
	ele = menu->GetElementById("height-input");
	Rml::ElementFormControlInput* heightElement = rmlui_dynamic_cast<Rml::ElementFormControlInput*>(ele);
	Vector size(std::stoi(widthElement->GetValue()), std::stoi(heightElement->GetValue()));

	// set all data at end
	circuit->setCircuitName(name);
	blockData->setSize(size);
}

void BlockCreationWindow::resetMenu() {
	// std::vector<std::pair<std::string, connection_end_id_t>> paths;
	// clear list
	while (list->GetNumChildren() > 0) list->RemoveChild(list->GetChild(0));
	Circuit* circuit = circuitViewWidget->getCircuitView()->getCircuit();
	if (!circuit) return;
	circuit_id_t id = circuit->getCircuitId();
	Rml::Element* ele = menu->GetElementById("name-input");
	Rml::ElementFormControlInput* nameElement = rmlui_dynamic_cast<Rml::ElementFormControlInput*>(ele);
	nameElement->SetValue(circuit->getCircuitName());
	
	const CircuitBlockData* circuitBlockData = circuitManager->getCircuitBlockDataManager()->getCircuitBlockData(id);
	if (!circuitBlockData) return;
	const BlockData* blockData = circuitManager->getBlockDataManager()->getBlockData(circuitBlockData->getBlockType());
	if (!blockData) return;

	ele = menu->GetElementById("width-input");
	Rml::ElementFormControlInput* widthElement = rmlui_dynamic_cast<Rml::ElementFormControlInput*>(ele);
	widthElement->SetValue(std::to_string(blockData->getSize().dx));
	ele = menu->GetElementById("height-input");
	Rml::ElementFormControlInput* heightElement = rmlui_dynamic_cast<Rml::ElementFormControlInput*>(ele);
	heightElement->SetValue(std::to_string(blockData->getSize().dy));

	const std::unordered_map<connection_end_id_t, std::pair<Vector, bool>>& conncections = blockData->getConnections();
	for (auto& iter : conncections) {
		connection_end_id_t endId = iter.first;
		bool isInputBool = iter.second.second;
		const Vector& positionOnBlock = iter.second.first;
		const std::string* connectionNamePtr = circuitBlockData->getConnectionIdToName(endId);
		std::string connectionName;
		if (connectionNamePtr) {
			connectionName = *connectionNamePtr;
		} else {
			connectionName = "None";
		}
		const Position* positionPtr = circuitBlockData->getConnectionIdToPosition(endId);
		Rml::ElementPtr row = document->CreateElement("div");
		// name
		Rml::XMLAttributes nameAttributes;
		nameAttributes["type"] = "text";
		nameAttributes["maxlength"] = "10";
		nameAttributes["size"] = "10";
		// nameAttributes["class"] = "name";
		Rml::ElementPtr name = Rml::Factory::InstanceElement(document, "input", "input", nameAttributes);
		Rml::ElementFormControlInput* nameElement = rmlui_dynamic_cast<Rml::ElementFormControlInput*>(name.get());
		nameElement->SetValue(connectionName);
		// isInput
		Rml::XMLAttributes isInputAttributes;
		isInputAttributes["type"] = "checkbox";
		if (isInputBool) isInputAttributes["checked"] = "";
		Rml::ElementPtr isInput = Rml::Factory::InstanceElement(document, "input", "input", isInputAttributes);
		// positionOnBlock
		Rml::XMLAttributes positionOnBlockAttributes;
		positionOnBlockAttributes["type"] = "text";
		positionOnBlockAttributes["maxlength"] = "3";
		positionOnBlockAttributes["size"] = "2";
		// positionOnBlockAttributes["class"] = "number";
		Rml::ElementPtr positionOnBlockX = Rml::Factory::InstanceElement(document, "input", "input", positionOnBlockAttributes);
		Rml::ElementPtr positionOnBlockY = Rml::Factory::InstanceElement(document, "input", "input", positionOnBlockAttributes);
		Rml::ElementFormControlInput* positionOnBlockXElement = rmlui_dynamic_cast<Rml::ElementFormControlInput*>(positionOnBlockX.get());
		Rml::ElementFormControlInput* positionOnBlockYElement = rmlui_dynamic_cast<Rml::ElementFormControlInput*>(positionOnBlockY.get());
		positionOnBlockXElement->SetValue(std::to_string(positionOnBlock.dx));
		positionOnBlockYElement->SetValue(std::to_string(positionOnBlock.dy));
		// position
		Rml::XMLAttributes positionAttributes;
		positionAttributes["type"] = "text";
		positionAttributes["maxlength"] = "6";
		positionAttributes["size"] = "5";
		// positionAttributes["class"] = "number-long";
		Rml::ElementPtr positionX = Rml::Factory::InstanceElement(document, "input", "input", positionAttributes);
		Rml::ElementPtr positionY = Rml::Factory::InstanceElement(document, "input", "input", positionAttributes);
		Rml::ElementFormControlInput* positionXElement = rmlui_dynamic_cast<Rml::ElementFormControlInput*>(positionX.get());
		Rml::ElementFormControlInput* positionYElement = rmlui_dynamic_cast<Rml::ElementFormControlInput*>(positionY.get());
		if (positionPtr) {
			positionXElement->SetValue(std::to_string(positionPtr->x));
			positionYElement->SetValue(std::to_string(positionPtr->y));
		} else {
			positionXElement->SetValue("N/A");
			positionYElement->SetValue("N/A");
		}
		// Rml::ElementPtr ele = document->CreateElement("div");
		row->AppendChild(std::move(name));
		// row->AppendChild(std::move(ele));
		// ele = document->CreateElement("div");
		row->AppendChild(std::move(isInput));
		// row->AppendChild(std::move(ele));
		// ele = document->CreateElement("div");
		row->AppendChild(std::move(positionOnBlockX));
		// row->AppendChild(std::move(ele));
		// ele = document->CreateElement("div");
		row->AppendChild(std::move(positionOnBlockY));
		// row->AppendChild(std::move(ele));
		// ele = document->CreateElement("div");
		row->AppendChild(std::move(positionX));
		// row->AppendChild(std::move(ele));
		// ele = document->CreateElement("div");
		row->AppendChild(std::move(positionY));
		// row->AppendChild(std::move(ele));
		row->SetClass("connection-list-item", true);
		row->SetId("ConnectionListItem Id: " + std::to_string(endId));
		list->AppendChild(std::move(row));
	}
}

void BlockCreationWindow::makePaths(std::vector<std::vector<std::string>>& paths, std::vector<std::string>& path, const AddressTreeNode<Evaluator::EvaluatorGate>& addressTree) {
	auto& branches = addressTree.getBranchs();
	if (branches.empty()) {
		paths.push_back(path);
	} else {
		for (auto& pair : branches) {
			path.push_back(circuitManager->getCircuit(pair.second.getContainerId())->getCircuitName() + pair.first.toString());
			makePaths(paths, path, pair.second);
			path.pop_back();
		}
	}
}

void BlockCreationWindow::updateSelected(std::string string) {
	std::vector<std::string> parts = stringSplit(string, '/');
	std::stringstream evalName(parts.front());
	std::string str;
	evaluator_id_t evalId;
	evalName >> str >> evalId;
	Address address;
	for (unsigned int i = 1; i < parts.size(); i++) {
		std::string part = parts[i];
		unsigned int index = part.size();
		while (index != 0 && part[index - 1] != '(') index--;
		std::stringstream posString(part.substr(index, part.size() - index - 1));
		Position position;
		char c;
		posString >> position.x >> c >> position.y;
		logInfo(position.toString());
		address.addBlockId(position);
	}

	CircuitView* circuitView = circuitViewWidget->getCircuitView();
	circuitView->getBackend()->linkCircuitViewWithEvaluator(circuitView, evalId, address);
}
