#include "undoTree.h"

// ================================================================================================
// Class UndoTree implementation

UndoTree::UndoTree():
    currentBranch(new Branch()) { }
UndoTree::~UndoTree() {
    Branch* mainBranch = currentBranch;
    currentBranch = nullptr;
    while (mainBranch->parentBranch != nullptr) {
        mainBranch = mainBranch->parentBranch;
    }
    delete mainBranch;
}

void UndoTree::insert_diff(DifferenceSharedPtr diff) {
    assert(diff != nullptr);
    currentBranch = currentBranch->add_diff(diff);
}
DifferenceSharedPtr UndoTree::undo() {
    DifferenceSharedPtr diff = currentBranch->currentDiff();
    currentBranch = currentBranch->ascend();
    if (diff == currentBranch->currentDiff()) {
        return DifferenceSharedPtr(new Difference);
    } else return diff;
}
DifferenceSharedPtr UndoTree::redo(int branch) {
    DifferenceSharedPtr diff = currentBranch->currentDiff();
    currentBranch = currentBranch->descend(branch);
    if (diff == currentBranch->currentDiff()) {
        return DifferenceSharedPtr(new Difference);
    } else return diff;
}

// ================================================================================================
// Class UndoTree::Branch implementation

UndoTree::Branch::Branch():
    parentBranch(nullptr), position(0) { }

UndoTree::Branch::Branch(DifferenceSharedPtr diff):
    parentBranch(nullptr), position(0) {
    tree.emplace_back(Node(diff));
}
UndoTree::Branch::Branch(Branch* parent, DifferenceSharedPtr diff):
    parentBranch(parent), position(0) {
    tree.emplace_back(Node(diff));
}
UndoTree::Branch::~Branch() {
    for (Node node : tree) {
        if (node.branches != nullptr) {
            delete node.branches;
            node.branches = nullptr;
        }
    }
}

DifferenceSharedPtr UndoTree::Branch::currentDiff() const {
    return tree[position].diff;
}

UndoTree::Branch* UndoTree::Branch::add_diff(DifferenceSharedPtr diff) {
    // If at the end of branch, extend branch
    if (atBranchEnd()) {
        tree.emplace_back(Node(diff));
        position = tree.size() -1;
        return this;

    // Otherwise create a new branch
    } else {
        if (tree[position].branches == nullptr) {
            tree[position].branches = new std::vector<Branch>;
        }
        tree[position].branches->emplace_back(Branch(this, diff));
        return &tree[position].branches->back();
    }
}
UndoTree::Branch* UndoTree::Branch::ascend() {
    if (!atBranchBegin()) {
        position--;
        return this;
    } else {
        if (parentBranch == nullptr) {
            return this;
        } else return parentBranch;
    }
}
UndoTree::Branch* UndoTree::Branch::descend(int branch) {
    if (atBranchEnd()) {
        return this;
    } else if (branch == -1) {
        position++;
        return this;
    } else return &(*tree[position].branches)[branch];
}

bool UndoTree::Branch::atBranchBegin() const {
    return position == 0;
}
bool UndoTree::Branch::atBranchEnd() const {
    bool atEnd = tree.empty();
    if (!atEnd) {
        atEnd = position == tree.size() - 1;
    }
    return atEnd;
}

// ================================================================================================
// Class UndoTree::Branch::Node implementation

UndoTree::Branch::Node::Node(DifferenceSharedPtr diff):
    diff(diff), branches(nullptr) { }

