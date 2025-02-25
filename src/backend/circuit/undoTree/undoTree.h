#ifndef undoTree_h
#define undoTree_h

#include "../../container/difference.h"

class UndoTree {
public:
    class iterator;
    UndoTree();
    ~UndoTree();

    size_t size() const;
    size_t numBranches() const;

    /**
     * @brief Inserts a diff to the UndoTree.
     *
     * Appends the diff to an existing branch if the iterator is at
     * the end of the branch. Otherwise, a new branch is created with
     * the new diff as the first element.
     *
     * @param it An iterator to the element the new diff will be inserted after.
     * @param diff A DifferenceSharedPtr to the new diff.
     * @returns An iterator to the inserted diff.
     */
    iterator insert(const iterator& it, DifferenceSharedPtr diff);

    void clear();
    void prune(const iterator& begin);
    void prune(const iterator& begin, const iterator& end);

    iterator begin();
    iterator end();
private:
    class Branch;
    Branch* mainBranch;
    std::unordered_set<Branch*> branches;
    size_t numNodes;
};

struct UndoTree::Branch {
    struct Node {
        UndoTree::Branch::Node(DifferenceSharedPtr diff);
        DifferenceSharedPtr diff;
        std::vector<Branch*>* branches;
    };

    Branch(UndoTree* tree);
    Branch(UndoTree* tree, Node* parent, DifferenceSharedPtr diff);

    UndoTree* tree;
    Node* parentNode;
    std::vector<Node> nodes;
};

class UndoTree::iterator {
    friend class UndoTree;
public:
    /**
     * @brief Goes to the next diff along the specified branch
     * @param branch The branch to move along
     * @returns An iterator to the next diff
     */
    iterator next(int branch = -1) const;

    /**
     * @brief Goes to the previous diff
     * @returns An iterator to the previous diff
     */
    iterator prev() const;
private:
    iterator(Branch* branch, Branch::Node* node);
    Branch* branch;
    Branch::Node* pt;
};

#endif
