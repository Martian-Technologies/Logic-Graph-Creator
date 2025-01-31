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
    void undo();
    void redo();
    void changeBranch(int i);

    // gui related info    
    const std::vector<std::vector<Node>>& getTree() const { return tree; }
    

private:

    // jagged array tree version
    std::unordered_map<int ,std::vector<int>> branches; // id matches to ints that are branches of node
    std::vector<std::vector<Node>> tree; // jagged array enjoyer
    
    int branchPosition;
    int nodePosition; // inside the branch itself
};

struct Node {
        Node(DifferenceSharedPtr diff, unsigned int id) : diff(diff), id(id) {}
        DifferenceSharedPtr diff;
        unsigned int id;
};

#endif /* treeSystem_h */
