#include "undoTree.h"

// ================================================================================================
// Class UndoTree implementation

UndoTree::UndoTree():
    mainBranch(new Branch(this)), numNodes(0) {
    branches.insert(mainBranch);
    mainBranch->nodes.emplace_back(DifferenceSharedPtr(new Difference));
}
UndoTree::~UndoTree() {
    for (Branch* branch : branches) {
        delete branch;
    }
    branches.clear();
    mainBranch = nullptr;
}

size_t UndoTree::size() const {
    return numNodes;
}
size_t UndoTree::numBranches() const {
    return branches.size();
}

UndoTree::iterator UndoTree::insert(const iterator& it, DifferenceSharedPtr diff) {
    // Make sure the iterator is on this tree
    assert(branches.contains(it.branch));

    // If the iterator is at the end of its branch append the new diff
    if (&it.branch->nodes.back() == it.pt) {
        it.branch->nodes.emplace_back(Branch::Node(diff));
        return iterator(it.branch, &it.branch->nodes.back());
    // If the iterator is not at the end of its branch make a new branch
    } else {
        Branch* newBranch = new Branch(this, it.pt, diff);
        branches.insert(newBranch);
        if (it.pt->branches == nullptr) {
            it.pt->branches = new std::vector<Branch*>;
        }
        it.pt->branches->push_back(newBranch);
        return iterator(newBranch, &newBranch->nodes.back());
    }
}

void UndoTree::clear() {
    for (Branch* branch : branches) {
        delete branch;
    }
    branches.clear();
    mainBranch = new Branch(this);
    mainBranch->nodes.emplace_back(DifferenceSharedPtr(new Difference));
    branches.insert(mainBranch);
    numNodes = 0;
}

// ================================================================================================
// Class UndoTree::Branch implementation

UndoTree::Branch::Branch(UndoTree* tree):
    tree(tree), parentNode(nullptr) { }
UndoTree::Branch::Branch(UndoTree* tree, Node* parent, DifferenceSharedPtr diff):
    tree(tree), parentNode(parent) {
    nodes.emplace_back(Node(diff));
}

// ================================================================================================
// Class UndoTree::Branch::Node implementation

UndoTree::Branch::Node::Node(DifferenceSharedPtr diff):
    diff(diff), branches(nullptr) { }
