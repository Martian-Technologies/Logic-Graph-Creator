#ifndef treeSystem_h
#define treeSystem_h

#include <deque>

#include "backend/container/difference.h"

struct Branch; 

class UndoTree {
public:
    UndoTree();
	~UndoTree();

	// actions
    void add(DifferenceSharedPtr difference);
    DifferenceSharedPtr undo();
    DifferenceSharedPtr redo();
    void changeBranch(int i);
private:
	Branch* currentBranch;
};

struct Branch {
	int index;
	Branch* parent;
	std::vector<std::pair<DifferenceSharedPtr, std::vector<std::unique_ptr<Branch>> > > nodes;
};

#endif /* treeSystem_h */
