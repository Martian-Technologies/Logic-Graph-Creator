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

    /**
     * @brief Clears the contents of the UndoTree.
     */
    void clear();

    /**
     * @brief Removes all elements and branches after a given element (inclusive).
     * @param begin An iterator to the first element to remove.
     */
    void prune(const iterator& begin);

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
        Node(DifferenceSharedPtr diff);
        ~Node();
        DifferenceSharedPtr diff;
        std::vector<Branch*>* branches;
    };

    Branch(UndoTree* tree);
    Branch(UndoTree* tree, Branch* parentBranch, int parentNode, DifferenceSharedPtr diff);

    UndoTree* tree;
    Branch* parentBranch;
    int parentNode;
    std::vector<Node> nodes;
};

class UndoTree::iterator {
    friend class UndoTree;
public:
    /**
     * @brief Goes to the next diff along the specified branch.
     * @param whichBranch The branch to move into.
     * @returns An iterator to the next diff.
     */
    iterator next(int whichBranch = -1) const;

    /**
     * @brief Goes to the previous diff.
     * @returns An iterator to the previous diff.
     */
    iterator prev() const;

    /**
     * @brief Finds the number of branches that split off from the current diff.
     */
    int numBranches() const;

    bool operator==(const iterator& other) const;
private:
    iterator(Branch* branch, int pos);
    Branch* branch;
    int pos;
};

#endif
