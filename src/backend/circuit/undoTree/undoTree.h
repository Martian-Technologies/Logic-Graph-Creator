#ifndef undoTree_h
#define undoTree_h

#include "../../container/difference.h"

class UndoTree {
public:
    class iterator;
    UndoTree();
    ~UndoTree();

    void insert_diff(DifferenceSharedPtr diff);
    DifferenceSharedPtr undo();
    DifferenceSharedPtr redo(int branch = -1);
private:
    class Branch;
    Branch* currentBranch;
};

struct UndoTree::Branch {
    struct Node {
        UndoTree::Branch::Node(DifferenceSharedPtr diff);
        DifferenceSharedPtr diff;
        std::vector<Branch>* branches;
    };

    Branch();
    Branch(DifferenceSharedPtr diff);
    Branch(Branch* parent, DifferenceSharedPtr diff);
    ~Branch();

    DifferenceSharedPtr currentDiff() const;

    Branch* add_diff(DifferenceSharedPtr diff);
    Branch* ascend();
    Branch* descend(int branch = -1);

    Branch* parentBranch;
    size_t position;
    std::vector<Node> tree;

    bool atBranchBegin() const;
    bool atBranchEnd() const;
};

class UndoTree::iterator {
public:

private:
    Branch::Node* pt;
};

#endif
