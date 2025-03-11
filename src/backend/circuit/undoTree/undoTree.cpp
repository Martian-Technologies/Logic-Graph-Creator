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
    if (it.pos == it.branch->nodes.size() - 1) {
        it.branch->nodes.emplace_back(Branch::Node(diff));
        return it.next();
    // If the iterator is not at the end of its branch make a new branch
    } else {
        Branch* newBranch = new Branch(this, it.branch, it.pos, diff);
        branches.insert(newBranch);
        if (it.branch->nodes[it.pos].branches == nullptr) {
            it.branch->nodes[it.pos].branches = new std::vector<Branch*>;
        }
        it.branch->nodes[it.pos].branches->push_back(newBranch);
        return iterator(newBranch, 0);
    }
    numNodes++;
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

void UndoTree::prune(const iterator& begin) {
    // Make sure the iterator is on this tree
    assert(begin.branch->tree == this);
    // Delete all branches that originate after begin
    for (int i = begin.pos; i < begin.branch->nodes.size(); i++) {
        if (begin.branch->nodes[begin.pos].branches != nullptr) {
            for (Branch* b : *begin.branch->nodes[begin.pos].branches) {
                branches.erase(b);
                numNodes -= b->nodes.size();
                delete b;
            }
        }
    }
    // Remove all diffs after begin
    numNodes -= begin.branch->nodes.size() - begin.pos;
    begin.branch->nodes.erase(begin.branch->nodes.begin() + begin.pos);
}

UndoTree::iterator UndoTree::begin() {
    return iterator(mainBranch, 0);
}

UndoTree::iterator UndoTree::end() {
    return iterator(mainBranch, -1);
}

// ================================================================================================
// Class UndoTree::iterator implementation

UndoTree::iterator::iterator(Branch* branch, int pos):
    branch(branch), pos(pos) { }

UndoTree::iterator UndoTree::iterator::next(int whichBranch) const {
    if (whichBranch == -1) {
        if (pos + 1 == this->branch->nodes.size()) {
            return iterator(this->branch, -1);
        } else return iterator(this->branch, pos + 1);
    } else return iterator((*this->branch->nodes[pos].branches)[whichBranch], 0);
}

UndoTree::iterator UndoTree::iterator::prev() const {
    if (pos == -1) {
        return iterator(this->branch, this->branch->nodes.size() - 1);
    } else if (pos == 0) {
        return iterator(this->branch->parentBranch, this->branch->parentNode);
    } else return iterator(this->branch, pos - 1);
}

int UndoTree::iterator::numBranches() const {
    if (branch->nodes[pos].branches == nullptr) {
        return 0;
    } else return branch->nodes[pos].branches->size();
}

bool UndoTree::iterator::operator==(const iterator& other) const {
    // -1 denotes end iterator
    if (this->pos == other.pos == -1) {
        return true;
    // Otherwise check if they are at same place on same branch
    } else return this->pos == other.pos && this->branch == other.branch;
}

bool UndoTree::iterator::operator!=(const iterator& other) const {
    return !(*this == other);
}

// ================================================================================================
// Class UndoTree::Branch implementation

UndoTree::Branch::Branch(UndoTree* tree):
    tree(tree), parentBranch(nullptr), parentNode(-1) { }
UndoTree::Branch::Branch(UndoTree* tree, Branch* parentBranch, int parentNode, DifferenceSharedPtr diff):
    tree(tree), parentBranch(parentBranch), parentNode(parentNode) {
    nodes.emplace_back(Node(diff));
}

// ================================================================================================
// Class UndoTree::Branch::Node implementation

UndoTree::Branch::Node::Node(DifferenceSharedPtr diff):
    diff(diff), branches(nullptr) { }

UndoTree::Branch::Node::~Node() {
    delete branches;
}
