#include "evalWindow.h"
#include "backend/dataUpdateEventManager.h"
#include "backend/evaluator/evaluatorManager.h"
#include "backend/circuit/circuitManager.h"
#include "util/algorithm.h"

EvalWindow::EvalWindow(
	const EvaluatorManager* evaluatorManager,
	const CircuitManager* circuitManager,
	DataUpdateEventManager* dataUpdateEventManager,
	Rml::ElementDocument* document,
	Rml::Element* parent
) : menuTree(document, parent, true, false), dataUpdateEventReceiver(dataUpdateEventManager), evaluatorManager(evaluatorManager), circuitManager(circuitManager) {
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
	logInfo(string);
	// std::vector parts = stringSplit(string, '/');
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
