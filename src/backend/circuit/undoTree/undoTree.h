#ifndef undoTree_h
#define undoTree_h

#include "../../container/difference.h"

class UndoTree {
public:
	class iterator;
	UndoTree();
	~UndoTree();

	size_t numBranches() const;
	iterator insert(const iterator& it, DifferenceSharedPtr diff);
	void clear();
	void prune(const iterator& begin);

	iterator begin();
	iterator end();
private:
	class Branch;
	Branch* mainBranch;
	std::unordered_set<Branch*> branches;
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
	friend bool onSameBranch(const UndoTree::iterator& a, const UndoTree::iterator& b);
public:
	iterator next(int whichBranch = -1) const;
	iterator prev() const;

	int numBranches() const;

	int whichBranch() const;

	DifferenceSharedPtr operator*() const;
	bool operator==(const iterator& other) const;
	bool operator!=(const iterator& other) const;
private:
	iterator(Branch* branch, int pos);
	Branch* branch;
	int pos;
};

#endif
