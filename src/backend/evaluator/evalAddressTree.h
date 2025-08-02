#ifndef evalAddressTree_h
#define evalAddressTree_h

#include "backend/address.h"

class EvalAddressTree {
public:
	EvalAddressTree() = default;
	EvalAddressTree(circuit_id_t containerId)
		: containerId(containerId) {}
	const std::unordered_map<Position, EvalAddressTree>& getBranches() const {
		return branches;
	}
	void addBranch(const Position& position, const EvalAddressTree& branch) {
		branches[position] = branch;
	}
	circuit_id_t getContainerId() const {
		return containerId;
	}
private:
	circuit_id_t containerId;
	std::unordered_map<Position, EvalAddressTree> branches;
};

#endif // evalAddressTree_h