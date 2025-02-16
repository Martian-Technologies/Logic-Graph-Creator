#include "treeSystem.h"
#include "backend/container/difference.h"

TreeManager::TreeManager() : branchPosition(0), nodePosition(0) {}

// actions
void TreeManager::add(DifferenceSharedPtr difference) {
    nodeCount++;
	branchHistory.clear();

    if (nodePosition < tree[branchPosition].size()) { // creates a new branch history if not at newest node
        tree.push_back({}); 

        // pairs id to new branch
        branchMap[nodeCount].push_back(std::make_pair(branchPosition, nodePosition)); // maps new node to where it branched off from
        branchMap[tree[branchPosition][nodePosition].id].push_back(tree.size() - 1, 0); // maps old node to branch off

        branchPosition = tree.size() - 1;
        nodePosition = 0;
    }

    tree[branchPosition].emplace_back(difference, nodeCount); // creates a new Node
    nodePosition++;
}

DifferenceSharedPtr TreeManager::undo() {
    if (nodePosition == 0) { // start of branch, must go to location branch branched off from
		branchHistory.push_back(branchPosition); // whenever changing branches, stores the value of the previous branch for going back upward
		branchPosition = branchMap[tree[branchPosition][nodePosition].id][0]->first;
		nodePosition = branchMap[tree[branchPosition][nodePosition].id][0]->second + 1;
    }

    return tree[branchPosition][nodePosition--].diff;
}

DifferenceSharedPtr TreeManager::redo() {
    // this wont redo to new trees
    if (nodePosition+1 == tree[branchPosition].size()){
		if(branchHistory.size()>0){
			branchPosition = branchHistory.front();
			branchHistory.pop_front();
			return tree[branchPosition][nodePosition++].diff;
		}
		return std::make_shared<Difference>();
	}
    return tree[branchPosition][nodePosition++].diff;
}

void TreeManager::changeBranch(int i) {
    branchPosition = i;
    nodePosition = 0;
}







