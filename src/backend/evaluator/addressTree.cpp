#include "addressTree.h"

template<class T>
void AddressTree<T>::addValue(block_id_t blockId, T leaf) {
    if (hasBranch(blockId) || hasLeaf(blockId)) {
        throw std::invalid_argument("AddressTree::addLeaf: blockId already exists");
    }
    leaves[blockId] = leaf;
}

template<class T>
void AddressTree<T>::addValue(const Address& address, T leaf) {
    // get the branch that the leaf should be added to: the address minus the last blockId. create any necessary branches, but make sure to not overwrite any existing leaves/branches
    AddressTree<T>* currentBranch = this;
    for (int i = 0; i < address.size() - 1; i++) {
        if (!currentBranch->hasBranch(address.getBlockId(i))) {
            currentBranch->branches[address.getBlockId(i)] = AddressTree<T>();
        }
        currentBranch = &currentBranch->branches[address.getBlockId(i)];
    }
    currentBranch->makeLeaf(address.getBlockId(address.size() - 1), leaf);
}

template<class T>
void AddressTree<T>::makeBranch(block_id_t blockId, AddressTree<T> branch) {
    if (hasBranch(blockId) || hasLeaf(blockId)) {
        throw std::invalid_argument("AddressTree::addBranch: blockId already exists");
    }
    branches[blockId] = branch;
}



template<class T>
AddressTree<T> AddressTree<T>::getBranch(const Address& address) const {
    AddressTree<T> currentBranch = *this;
    for (int i = 0; i < address.size(); i++) {
        const auto pt = currentBranch.branches.find(address.getBlockId(i));
        if (pt == currentBranch.branches.end()) {
            throw std::out_of_range("AddressTree::getBranch: address not found");
        }
        currentBranch = pt->second;
    }
    return currentBranch;
}

template<class T>
T AddressTree<T>::getLeaf(const Address& address) const {
    AddressTree<T> currentBranch = *this;
    for (int i = 0; i < address.size() - 1; i++) {
        const auto pt = currentBranch.branches.find(address.getBlockId(i));
        if (pt == currentBranch.branches.end()) {
            throw std::out_of_range("AddressTree::getLeaf: address not found");
        }
        currentBranch = pt->second;
    }
    return currentBranch.getLeaf(address.getBlockId(address.size() - 1));
}

template<class T>
void AddressTree<T>::remap(const std::unordered_map<T, T>& mapping) {
    for (auto& [key, value] : leaves) {
        if (mapping.find(value) != mapping.end()) {
            value = mapping.at(value);
        }
    }
    for (auto& [key, value] : branches) {
        value.remap(mapping);
    }
}