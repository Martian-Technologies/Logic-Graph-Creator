#ifndef treeSystem_h
#define treeSystem_h

#include "backend/container/difference.h"
#include <unordered_map>

struct Node; 

class TreeManager {
public:
    TreeManager();

    // actions
    void add(DifferenceSharedPtr difference);
    DifferenceSharedPtr undo();
    DifferenceSharedPtr redo();
    void changeBranch(int i);

    // gui related info    
    const std::vector<std::vector<Node>>& getTree() const { return tree; }
    std::vector<unsigned int> getBranchConnection(const unsigned int i) const { return branchMap.find(i)->second; }
    

private:

    // jagged array tree version
    std::unordered_map<int ,std::vector<unsigned int>> branchMap; // id matches to ints that are branches of node
    std::vector<std::vector<Node>> tree; // jagged array enjoyer
    
    unsigned int branchPosition;
    unsigned int nodePosition; // inside the branch itself
    unsigned int nodeCount;
};

struct Node {
        Node(DifferenceSharedPtr diff, unsigned int id) : diff(diff), id(id), branching(0) {}
        DifferenceSharedPtr diff;
        unsigned int id; // node count correlates to branches map, it will pair id to what branches said node connects to
        bool branching; // makes it easier then searching unordered map on whether or not this node branches off
};

#endif /* treeSystem_h */
