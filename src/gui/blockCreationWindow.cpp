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
	resetButton->AddEventListener("click", new EventPasser(std::bind(&BlockCreationWindow::resetMenu, this)));
	// add
	Rml::Element* addConnection = menu->GetElementById("connection-list-add");
	addConnection->AddEventListener("click", new EventPasser(std::bind(&BlockCreationWindow::addListItem, this)));
	// dataUpdateEvents
	dataUpdateEventReceiver.linkFunction("blockDataUpdate", std::bind(&BlockCreationWindow::resetMenu, this));
	dataUpdateEventReceiver.linkFunction("circuitViewChangeCircuit", std::bind(&BlockCreationWindow::resetMenu, this));
	dataUpdateEventReceiver.linkFunction("blockDataSetConnection", std::bind(&BlockCreationWindow::resetMenu, this));
	dataUpdateEventReceiver.linkFunction("blockDataRemoveConnection", std::bind(&BlockCreationWindow::resetMenu, this));
	dataUpdateEventReceiver.linkFunction("circuitBlockDataConnectionPositionSet", std::bind(&BlockCreationWindow::resetMenu, this));
	resetMenu();
}

void BlockCreationWindow::updateFromMenu() {
	Circuit* circuit = circuitViewWidget->getCircuitView()->getCircuit();
	if (!circuit) return;
	circuit_id_t id = circuit->getCircuitId();
	CircuitBlockData* circuitBlockData = circuitManager->getCircuitBlockDataManager()->getCircuitBlockData(id);
	BlockData* blockData = circuitManager->getBlockDataManager()->getBlockData(circuitBlockData->getBlockType());
	std::string name;
	Vector size;
	std::vector<std::tuple<connection_end_id_t, std::string, bool, Vector, Position>> portsData;
	try {

		// we dont update till the end because setting data will cause the UI to update

		Rml::Element* ele = menu->GetElementById("name-input");
		Rml::ElementFormControlInput* nameElement = rmlui_dynamic_cast<Rml::ElementFormControlInput*>(ele);
		name = nameElement->GetValue();

		if (!blockData || !circuitBlockData) {
			// set data if nothing more to get
			circuit->setCircuitName(name);
			return;
		}

		ele = menu->GetElementById("width-input");
		Rml::ElementFormControlInput* widthElement = rmlui_dynamic_cast<Rml::ElementFormControlInput*>(ele);
		ele = menu->GetElementById("height-input");
		Rml::ElementFormControlInput* heightElement = rmlui_dynamic_cast<Rml::ElementFormControlInput*>(ele);
		size = Vector(std::stoi(widthElement->GetValue()), std::stoi(heightElement->GetValue()));

		for (unsigned int i = 0; i < list->GetNumChildren(); i++) {
			Rml::Element* row = list->GetChild(i);
			const std::string& rowId = row->GetId();
			// get connection end id
			connection_end_id_t endId = std::stoi(rowId.substr(23, rowId.size() - 23));
			// get port name
			Rml::ElementList elements;
			row->GetElementsByClassName(elements, "connection-list-item-name");
			const std::string& portName = rmlui_dynamic_cast<Rml::ElementFormControlInput*>(elements.front())->GetValue();
			// get port is input
			elements.clear();
			row->GetElementsByClassName(elements, "connection-list-item-is-input");
			bool portIsInput = rmlui_dynamic_cast<Rml::ElementFormControlInput*>(elements.front())->HasAttribute("checked");
			// get port position on block
			Vector portPositionOnBlock;
			elements.clear();
			row->GetElementsByClassName(elements, "connection-list-item-on-block-x");
			portPositionOnBlock.dx = std::stoi(rmlui_dynamic_cast<Rml::ElementFormControlInput*>(elements.front())->GetValue());
			elements.clear();
			row->GetElementsByClassName(elements, "connection-list-item-on-block-y");
			portPositionOnBlock.dy = std::stoi(rmlui_dynamic_cast<Rml::ElementFormControlInput*>(elements.front())->GetValue());
			// get port block position
			Position portBlockPosition;
			elements.clear();
			row->GetElementsByClassName(elements, "connection-list-item-pos-x");
			portBlockPosition.x = std::stoi(rmlui_dynamic_cast<Rml::ElementFormControlInput*>(elements.front())->GetValue());
			elements.clear();
			row->GetElementsByClassName(elements, "connection-list-item-pos-y");
			portBlockPosition.y = std::stoi(rmlui_dynamic_cast<Rml::ElementFormControlInput*>(elements.front())->GetValue());

			portsData.emplace_back(endId, portName, portIsInput, portPositionOnBlock, portBlockPosition);
		}
	} catch (const std::exception& e) {
		// Top level fatal error catcher, logs issue
		logError("{}", "", e.what());
		return;
	}

	// set all data at end
	circuit->setCircuitName(name);
	blockData->setSize(size);

	for (auto row : portsData) {
		connection_end_id_t endId = std::get<0>(row);
		std::string portName = std::get<1>(row);
		bool portIsInput = std::get<2>(row);
		Vector portPositionOnBlock = std::get<3>(row);
		Position portBlockPosition = std::get<4>(row);
		if (blockData->connectionExists(endId)) {
			if (blockData->isConnectionInput(endId) != portIsInput) blockData->removeConnection(endId);
		}

		if (portIsInput) blockData->setConnectionInput(portPositionOnBlock, endId);
		else blockData->setConnectionOutput(portPositionOnBlock, endId);

		if (!(portName.empty())) circuitBlockData->setConnectionIdName(endId, portName);
		circuitBlockData->setConnectionIdName(endId, portName);
		circuitBlockData->setConnectionIdPosition(endId, portBlockPosition);
	}
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
			connectionName = "";
		}
		const Position* positionPtr = circuitBlockData->getConnectionIdToPosition(endId);
		Rml::ElementPtr row = document->CreateElement("div");
		// name
		Rml::XMLAttributes nameAttributes;
		nameAttributes["type"] = "text";
		nameAttributes["maxlength"] = "10";
		nameAttributes["size"] = "10";
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
		row->AppendChild(std::move(name))->SetClass("connection-list-item-name", true);
		row->AppendChild(std::move(isInput))->SetClass("connection-list-item-is-input", true);
		row->AppendChild(std::move(positionOnBlockX))->SetClass("connection-list-item-on-block-x", true);
		row->AppendChild(std::move(positionOnBlockY))->SetClass("connection-list-item-on-block-y", true);
		row->AppendChild(std::move(positionX))->SetClass("connection-list-item-pos-x", true);
		row->AppendChild(std::move(positionY))->SetClass("connection-list-item-pos-y", true);
		row->SetClass("connection-list-item", true);
		row->SetId("ConnectionListItem Id: " + std::to_string(endId));
		list->AppendChild(std::move(row));
	}
}

void BlockCreationWindow::addListItem() {
	Circuit* circuit = circuitViewWidget->getCircuitView()->getCircuit();
	if (!circuit) return;
	circuit_id_t id = circuit->getCircuitId();
	const CircuitBlockData* circuitBlockData = circuitManager->getCircuitBlockDataManager()->getCircuitBlockData(id);
	if (!circuitBlockData) return;
	const BlockData* blockData = circuitManager->getBlockDataManager()->getBlockData(circuitBlockData->getBlockType());
	if (!blockData) return;

	connection_end_id_t endId = 0;
	while (true) {
		while (blockData->connectionExists(endId)) ++endId;
		if (list->GetElementById("ConnectionListItem Id: " + std::to_string(endId)) == nullptr) break;
		++endId;
	}

	Rml::ElementPtr row = document->CreateElement("div");
	// name
	Rml::XMLAttributes nameAttributes;
	nameAttributes["type"] = "text";
	nameAttributes["maxlength"] = "10";
	nameAttributes["size"] = "10";
	Rml::ElementPtr name = Rml::Factory::InstanceElement(document, "input", "input", nameAttributes);
	Rml::ElementFormControlInput* nameElement = rmlui_dynamic_cast<Rml::ElementFormControlInput*>(name.get());
	nameElement->SetValue("");
	// isInput
	Rml::XMLAttributes isInputAttributes;
	isInputAttributes["type"] = "checkbox";
	isInputAttributes["checked"] = "";
	Rml::ElementPtr isInput = Rml::Factory::InstanceElement(document, "input", "input", isInputAttributes);
	// positionOnBlock
	Rml::XMLAttributes positionOnBlockAttributes;
	positionOnBlockAttributes["type"] = "text";
	positionOnBlockAttributes["maxlength"] = "3";
	positionOnBlockAttributes["size"] = "2";
	Rml::ElementPtr positionOnBlockX = Rml::Factory::InstanceElement(document, "input", "input", positionOnBlockAttributes);
	Rml::ElementPtr positionOnBlockY = Rml::Factory::InstanceElement(document, "input", "input", positionOnBlockAttributes);
	Rml::ElementFormControlInput* positionOnBlockXElement = rmlui_dynamic_cast<Rml::ElementFormControlInput*>(positionOnBlockX.get());
	Rml::ElementFormControlInput* positionOnBlockYElement = rmlui_dynamic_cast<Rml::ElementFormControlInput*>(positionOnBlockY.get());
	positionOnBlockXElement->SetValue(std::to_string(0));
	positionOnBlockYElement->SetValue(std::to_string(0));
	// position
	Rml::XMLAttributes positionAttributes;
	positionAttributes["type"] = "text";
	positionAttributes["maxlength"] = "6";
	positionAttributes["size"] = "5";
	Rml::ElementPtr positionX = Rml::Factory::InstanceElement(document, "input", "input", positionAttributes);
	Rml::ElementPtr positionY = Rml::Factory::InstanceElement(document, "input", "input", positionAttributes);
	Rml::ElementFormControlInput* positionXElement = rmlui_dynamic_cast<Rml::ElementFormControlInput*>(positionX.get());
	Rml::ElementFormControlInput* positionYElement = rmlui_dynamic_cast<Rml::ElementFormControlInput*>(positionY.get());
	positionXElement->SetValue("N/A");
	positionYElement->SetValue("N/A");
	row->AppendChild(std::move(name))->SetClass("connection-list-item-name", true);
	row->AppendChild(std::move(isInput))->SetClass("connection-list-item-is-input", true);
	row->AppendChild(std::move(positionOnBlockX))->SetClass("connection-list-item-on-block-x", true);
	row->AppendChild(std::move(positionOnBlockY))->SetClass("connection-list-item-on-block-y", true);
	row->AppendChild(std::move(positionX))->SetClass("connection-list-item-pos-x", true);
	row->AppendChild(std::move(positionY))->SetClass("connection-list-item-pos-y", true);
	row->SetClass("connection-list-item", true);
	row->SetId("ConnectionListItem Id: " + std::to_string(endId));
	list->AppendChild(std::move(row));
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
