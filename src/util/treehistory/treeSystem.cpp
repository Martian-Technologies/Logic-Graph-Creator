#include "treeSystem.h"

UndoTree::UndoTree() : currentBranch(new Branch()) { currentBranch->parent = nullptr; currentBranch->index = 0; }
UndoTree::~UndoTree() { delete currentBranch; }

void UndoTree::add(DifferenceSharedPtr difference) {	
	if (currentBranch->index < currentBranch->nodes.size()-1) {
		Branch* tmp = currentBranch;
		tmp->nodes[index]->second.push_back(std::make_unique<Branch>());
		currentBranch = tmp->nodes[index]->second[tmp->nodes[index]->second.size() - 1];
		currentBranch->index = 0;
		currentBranch->parent = tmp;
	}
	currentBranch->index++;
	currentBranch->nodes.emplace_back(difference, {});
}

DifferenceSharedPtr UndoTree::undo() {
	if (currentBranch->index == 0) {
		currentBranch = currentBranch->parent;
		return currentBranch->nodes[currentBranch->index]->first;
	}
	return currentBranch->nodes[currentBranch->index--]->first;
}

DifferenceSharedPtr UndoTree::redo() {
	if (currentBranch->index == currentBranch->nodes.size() - 1) {
		return std::make_shared<Difference>();
	}
	return currentBranch->nodes[currentBranch->index++]->first;
}



