#include "treeSystem.h"
#include "backend/container/difference.h"

TreeManager::TreeManager() : branchPosition(0), nodePosition(0) {}

// actions
void TreeManager::add(DifferenceSharedPtr difference) {
    nodeCount++;

    if (nodePosition < tree[branchPosition].size()) { // creates a new branch history if not at newest node
        tree.push_back({}); 

        // pairs id to new branch
        branchMap[nodeCount].push_back(nodePosition); // maps new node to where it branched off from
        branchMap[tree[branchPosition][nodePosition].id].push_back(0); // maps old node to branch off

        branchPosition = tree.size() - 1;
        nodePosition = 0;
    }
    tree[branchPosition].emplace_back(difference, nodeCount);
    nodePosition++;
}

DifferenceSharedPtr TreeManager::undo() {
    if (nodePosition == 0) {
        return NULL;
    }
    return tree[branchPosition][nodePosition--].diff;
}

DifferenceSharedPtr TreeManager::redo() {
    // this wont redo to new trees
    if (nodePosition+1 == tree[branchPosition].size())
        return std::make_shared<Difference>();
    return tree[branchPosition][nodePosition++].diff;
}

void TreeManager::changeBranch(int i) {
    branchPosition = i;
    nodePosition = 0;
}







