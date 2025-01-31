#ifndef treeSystem_h
#define treeSystem_h

#include "backend/container/difference.h"

class TreeManager {
public:
    TreeManager();

    // actions
    void add(DifferenceSharedPtr difference);
    void undo();
    void redo();
    void changeBranch(int i);

    // cleanup
    // need to remove branches
    
    

private:

    struct Node {
        Node(DifferenceSharedPtr diff, unsigned int id) : diff(diff), id(id) {}
        DifferenceSharedPtr diff;
        unsigned int id;
    };

    // offset tree method
    std::vector<std::vector<int>> branches; // id matches to ints that are branches of node
    std::vector<std::vector<Node>> tree; // jagged array enjoyer
};


#endif /* treeSystem_h */
