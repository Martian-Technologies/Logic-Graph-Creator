#include "evalWindow.h"
#include "backend/evaluator/evaluatorManager.h"
#include "backend/dataUpdateEventManager.h"
#include "backend/circuit/circuitManager.h"
#include "circuitViewWidget.h"
#include "backend/backend.h"
#include "util/algorithm.h"

EvalWindow::EvalWindow(
	const EvaluatorManager* evaluatorManager,
	const CircuitManager* circuitManager,
	std::shared_ptr<CircuitViewWidget> circuitViewWidget,
	DataUpdateEventManager* dataUpdateEventManager,
	Rml::ElementDocument* document,
	Rml::Element* parent
) : menuTree(document, parent, true, false), dataUpdateEventReceiver(dataUpdateEventManager), evaluatorManager(evaluatorManager), circuitManager(circuitManager), circuitViewWidget(circuitViewWidget) {
	dataUpdateEventReceiver.linkFunction("addressTreeMakeBranch", std::bind(&EvalWindow::updateList, this));
	menuTree.setListener(std::bind(&EvalWindow::updateSelected, this, std::placeholders::_1));
	updateList();
}

void EvalWindow::updateList() {
	std::vector<std::vector<std::string>> paths;
	for (auto pair : evaluatorManager->getEvaluators()) {
		std::vector<std::string> path({ pair.second->getEvaluatorName() });
		makePaths(paths, path, pair.second->getAddressTree());
	}
	menuTree.setPaths(paths);
}

void EvalWindow::makePaths(std::vector<std::vector<std::string>>& paths, std::vector<std::string>& path, const AddressTreeNode<Evaluator::EvaluatorGate>& addressTree) {
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

void EvalWindow::updateSelected(std::string string) {
	std::vector<std::string> parts = stringSplit(string, '/');
	std::stringstream evalName(parts.front());
	std::string str;
	evaluator_id_t evalId;
	evalName >> str >> evalId;
	Address address;
	for (unsigned int i = 1; i < parts.size(); i++) {
		std::string part = parts[i];
		unsigned int index = part.size();
		while (index != 0 && part[index-1] != '(') index--;
		std::stringstream posString(part.substr(index, part.size()-index-1));
		Position position;
		char c;
		posString >> position.x >> c >> position.y;
		logInfo(position.toString());
		address.addBlockId(position);
	}

	CircuitView* circuitView = circuitViewWidget->getCircuitView();
	circuitView->getBackend()->linkCircuitViewWithEvaluator(circuitView, evalId, address);

	// if (parts.size() <= 1) return;
	// if (parts[0] == "Blocks") {
	// 	BlockType blockType = blockDataManager->getBlockType(string.substr(7, string.size() - 7));
	// 	toolManagerManager->setBlock(blockType);
	// } else if (parts[0] == "Tools") {
	// 	toolManagerManager->setTool(string.substr(6, string.size() - 6));
	// } else {
	// 	logError("Do not recognize cadegory {}", "EvalWindow", parts[0]);
	// }
}

void EvalWindow::updateSelectedMode() {
	// if (!current || (previous && current->text() == previous->text())) return;
	// emit selectedModeChange(current->text().toStdString());
}

void EvalWindow::updateToolModeOptions(const std::vector<std::string>* modes) {

}
